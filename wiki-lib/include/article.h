/*
 * Copyright (c) 2009 Holger Hans Peter Freyther <zecke@openmoko.org>
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

#ifndef ARTICLE_H
#define ARTICLE_H

#include <wikilib.h>

enum article_nav {
	ARTICLE_PAGE_0 = 0,
	ARTICLE_PAGE_NEXT,
	ARTICLE_PAGE_PREV
};

/**
 * Return < 0 in case of error
 */
int	article_open(const char *article);
void	article_display(enum article_nav nav);
void	article_close(void);
void	article_extract_file_and_offset(const char *target, unsigned int *file, unsigned int *offset);

#endif
