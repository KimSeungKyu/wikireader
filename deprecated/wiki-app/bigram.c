/*
 * Copyright (c) 2009 Openmoko Inc.
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

#include <string.h>
#include <stdlib.h>
#include "msg.h"
#include "bigram.h"
#include "lcd_buf_draw.h"
#include "file-io.h"
#include "wiki_info.h"
#include <malloc-simple.h>

typedef struct _bitram {
	char char_pairs[128][2];
} BIGRAM, *PBIGRAM;
PBIGRAM aBigram = NULL;

int32_t aCharIdx[128];

void init_bigram(int fd)
{
	if (!aBigram)
	{
		aBigram = (PBIGRAM)malloc_simple(sizeof(BIGRAM) * get_wiki_count(), MEM_TAG_INDEX_M1);
	}
	init_char_idx();
	wl_read(fd, &aBigram[nCurrentWiki], sizeof(BIGRAM));
}

void init_char_idx()
{
	char c;
	int i;
	int idx = 1;
	static int inited = 0;

	if (inited)
		return;

	inited = 1;
	memset(aCharIdx, 0, sizeof(aCharIdx));
	for (i = 0; i < 128; i++)
	{
		c = (char)i;
		if (is_supported_search_char(c))
		{
			if ('a' <= c && c <= 'z')
				aCharIdx[i] = aCharIdx[(int)'A' + (c - 'a')];
			else
				aCharIdx[i] = idx++;
		}
	}
}

int bigram_char_idx(char c)
{
	return aCharIdx[(int)c];
}

void bigram_decode(char *outStr, char *inStr, int lenMax)
{
	unsigned char c;

	while (lenMax > 1 && (c = *inStr++) != '\0')
	{
		if (c >= 128 && aBigram[nCurrentWiki].char_pairs[c-128][0])
		{
			*outStr = aBigram[nCurrentWiki].char_pairs[c-128][0];
			outStr++;
			lenMax--;
			if (lenMax > 1)
			{
				*outStr = aBigram[nCurrentWiki].char_pairs[c-128][1];
				outStr++;
				lenMax--;
			}
		}
		else
		{
			*outStr = c;
			outStr++;
			lenMax--;
		}
	}
	*outStr = '\0';
}

int is_supported_search_char(char c)
{
	if (c && (strchr(SUPPORTED_SEARCH_CHARS, c) || ('A' <= c && c <= 'Z')))
		return 1;
	else
		return 0;
}

int search_string_cmp(char *title, char *search, int len)  // assuming search consists of lowercase only
{
	int rc = 0;
	char c = 0;

	while (!rc && len > 0)
	{
		c = *title;
		if (c && !is_supported_search_char(c))
		{
			title++;
		}
		else
		{
			if ('A' <= c && c <= 'Z')
				c += 32;
			if (c == *search)
			{
				title++;
				search++;
				len--;
			}
			else if (c > *search)
				rc = 1;
			else
				rc = -1;
		}
	}
	return rc;
}
