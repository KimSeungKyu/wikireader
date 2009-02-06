/*
 * mahatma - a simple kernel framework
 * Copyright (c) 2008, 2009 Daniel Mack <daniel@caiaq.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <guilib.h>
#include <wikilib.h>
#include <input.h>

#include <msg.h>
#include "regs.h"
#include "msg-output.h"
#include "touchscreen.h"
#include "serial.h"
#include "irq.h"

#define BUFSIZE 10

static unsigned int console_buf[BUFSIZE];
static unsigned int console_read = 0;
static unsigned int console_write = 0;

int serial_transfer_running(int port)
{
	switch (port) {
	case 0:
		return (REG_EFSIF0_STATUS >> 5) & 1;
	case 1:
		return (REG_EFSIF1_STATUS >> 5) & 1;
	default:
		return 0;
	}
}

void serial_init(void)
{
	//REG_INT_ESIF01 = 0x36;
	REG_INT_ESIF01 = 0x6;
	REG_INT_PLCDC_PSIO0 = 0x70;
	console_read = 0;
	console_write = 0;
}

void serial_check(void)
{
	char c;

	/* serial 0 in */
	while (REG_EFSIF0_STATUS & 0x1) {
		c = REG_EFSIF0_RXD;
		if (c == 0)
			continue;

		console_buf[console_write] = c;
		console_write++;
		console_write %= BUFSIZE;
	}

	/* serial 1 in */
	while (REG_EFSIF1_STATUS & 0x1) {
		touchscreen_read_char(REG_EFSIF1_RXD);
	}

	/* serial 0 out */
	if ((REG_EFSIF0_STATUS & 0x2) && get_msg_char(&c))
		REG_EFSIF0_TXD = c;
}

void serial_out(int port, char c)
{
	if (port != 0)
		return;

	REG_EFSIF0_TXD = c;
}

int serial_get_event(struct wl_input_event *ev)
{
	serial_check();

	if (console_read == console_write)
		return 0;

//	msg(MSG_INFO, " OUT. %d %d    %p %p\n", console_read, console_write,
//		ev, &ev->type);

	ev->type = WL_INPUT_EV_TYPE_KEYBOARD;
	ev->key_event.keycode = console_buf[console_read];
	ev->key_event.value = 1;
	console_read++;
	console_read %= BUFSIZE;
	return 1;
}

