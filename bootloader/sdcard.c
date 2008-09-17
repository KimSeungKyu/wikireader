/*
    e07 bootloader suite
    Copyright (c) 2008 Daniel Mack <daniel@caiaq.de>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * support code for SD cards, SDSC and SDHC. Some hints were taken from
 * the vector06cc project, others were reverse-enginered, guessed and
 * estimated due to dorky SD card consortium restrictions for
 * documentation. Shame on you.
 * 
 * Limitations of this stack:
 * 	* does not support MMC cards
 * 	* read-only, not write support
 * 	* support for one card only
 */

#include "regs.h"
#include "types.h"
#include "wikireader.h"
#include "spi.h"
#include "misc.h"
#include "crc.h"
#include "sdcard.h"

#define CMD_GO_IDLE_STATE	0x00
#define CMD_8			0x08
#define CMD_READ_CSD		0x09
#define CMD_READ_SECTOR		0x11
#define CMD_SEND_OP_COND	0x29
#define CMD_APP			0x37
#define CMD_41			41
#define CMD_58			58

#define MODE_SDSC 0
#define MODE_SDHC 1
#define MODE_NONE 0xff
static u8 mode;

static u8 sdcard_response(void)
{
	u8 ret, retry = 100;

	do {
		SDCARD_CS_LO();
		ret = spi_transmit(0xff);
		SDCARD_CS_HI();
	} while (--retry && (ret & 0x80));

	/* some SDHC cards seem to transmit 0x1f followed by the actual
	 * status code. */
	if (ret == 0x1f) {
		SDCARD_CS_LO();
		ret = spi_transmit(0xff);
		SDCARD_CS_HI();
	}

	return ret;
}

static void sdcard_cmd(u8 cmd, u32 param)
{
	u8 crc, i, c[] = {
		0x40 | cmd,
		param >> 24,
		param >> 16,
		param >> 8,
		param
	};

	SDCARD_CS_LO();
	spi_transmit(0xff);

	for (i = 0; i < sizeof(c); i++)
		spi_transmit(c[i]);

	/* checksum */
	/* FIXME: for some reason, the crc algorithm is wrong
	 * for CMD8. Hence the hack here. */
	switch (cmd) {
		case 0:
			crc = 0x95;
			break;
		case 8:
			crc = 0x87;
			break;
		default:
			crc = (crc7(c, sizeof(c)) << 1) | 1;
			break;
	}

	spi_transmit(crc);
	SDCARD_CS_HI();
}

int sdcard_read_sector(u32 sector, u8 *buf)
{
	u8 ret;
	u32 i, retry;

	//print("read sector "); print_u32(sector); print("\n");

	if (mode == MODE_SDSC)
		sector *= BYTES_PER_SECTOR;

	for (i = 0; i < BYTES_PER_SECTOR; i++)
		buf[i] = 0;

	sdcard_cmd(CMD_READ_SECTOR, sector);
	ret = sdcard_response();

	if (ret != 0) {
		print("bad card response: ");
		print_u32(ret); print("\n");
		return -1;
	}

	for (retry = 0; retry < 10000; retry++) {
		SDCARD_CS_LO();
		ret = spi_transmit(0xff);
		SDCARD_CS_HI();

		if (ret == 0xfe)
			break;
	}

	if (ret != 0xfe) {
		print("read timeout\n");
		return -1;
	}

	SDCARD_CS_LO();
	for (i = 0; i < BYTES_PER_SECTOR; i++)
		buf[i] = spi_transmit(0xff);

	/* swallow checksum */
	spi_transmit(0xff);
	spi_transmit(0xff);
	SDCARD_CS_HI();

	return 0;
}

static int sdcard_go_idle(void)
{
	u32 retry;
	u8 ret;

	for (retry = 100; retry; retry--) {
		sdcard_cmd(CMD_GO_IDLE_STATE, 0);
		ret = sdcard_response();

		if (ret == 0x01)
			break;
	}

	if (ret != 0x01) {
		print("unable to set SD card to IDLE state\n");
		SDCARD_CS_HI();
		return -1;
	}

	return 0;
}

static int sdcard_cmd8(u32 param)
{
	u32 retry;
	u8 ret;

	for (retry = 100; retry; retry--) {
		sdcard_cmd(CMD_8, param);
		ret = sdcard_response();
		if (ret == 0x01)
			break;

		delay(10000);
	}

	if (ret != 0x01) {
		print("unable to set card voltage conditions.\n");
		print("This card does not support SD2.0 spec.\n");
		SDCARD_CS_HI();
		return -1;
	}

	return 0;
}

static int sdcard_read_ccs(void)
{
	u32 retry;
	u8 ret;

	/* leave the IDLE state (CMD41 | HCS bit) */
	for (retry = 1000; retry; retry--) {
		sdcard_cmd(CMD_APP, 0);
		ret = sdcard_response();

		sdcard_cmd(CMD_41, (1UL << 30));
		ret = sdcard_response();

		if (ret == 0x00)
			break;

		delay(10000);
	}

	if (ret != 0x00) {
		print("Unable to set HCS idle state\n");
		return -1;
	}

	return 0;
}

int sdcard_init(void)
{
	u8 buf[BYTES_PER_SECTOR];
	u32 retry;

	disable_card_power();
	delay(100000);
	enable_card_power();

	/* 80 dummy clocks */
	SDCARD_CS_LO();
	for (retry = 0; retry < 10; retry++)
		spi_transmit(0xff);

	SDCARD_CS_LO();

	/* set card to IDLE state */
	if (sdcard_go_idle() < 0)
		return -1;

	/* send CMD8, interface voltage conditions. */
	if (sdcard_cmd8(0x1aa) < 0)
		return -1;
	
	if (sdcard_read_ccs() < 0)
		return -1;

	SDCARD_CS_HI();

	mode = MODE_SDHC;

	if (sdcard_read_sector(1, buf) < 0) {
		print("SD card unable to read sector, SDSC card?\n");
		return -1;
	}

	print("SD card initialized.\n");
	return 0;
}

