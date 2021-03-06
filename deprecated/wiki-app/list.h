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

/*
 *	Simple doubly linked list.
 *
 *  This is a generic implementation.
 *	No dynamic memeory allocation required.
 *
 */

#ifndef LIST_H_
#define LIST_H_

/*
 * Use this struct in the beginning of your structure declaration.
 */
struct wl_list {
	struct wl_list *prev, *next;
};

static inline void wl_list_init(struct wl_list *list)
{
	list->prev = list;
	list->next = list;
}

static inline void
wl_list_insert_after(struct wl_list *list, struct wl_list *node)
{
	node->prev = list;
	node->next = list->next;
	list->next->prev = node;
	list->next = node;
}


static inline void
wl_list_link_node(struct wl_list *prev, struct wl_list *next)
{
	prev->next = next;
	next->prev = prev;
}

static inline void wl_list_del(struct wl_list *node)
{
	wl_list_link_node(node->prev, node->next);
	wl_list_init(node);
}

static inline unsigned int wl_list_size(struct wl_list *list)
{
	struct wl_list *node = list->next;
	unsigned int count = 0;

	while (node != list) {
		++count;
		node = node->next;
	}

	return count;
}

static struct wl_list *
wl_list_search(struct wl_list *head, const void *value, unsigned int offset,
	       int (*comp)(const void *value, unsigned int offset, const struct wl_list *node))
{
	struct wl_list *p = head->next;

	while (p != head) {
		if (!(comp)(value, offset, p))
			break;
		p = p->next;
	}
	return p == head ? NULL : p;
}

static struct wl_list * wl_list_find_nth_node(struct wl_list *list, unsigned int nth)
{
	struct wl_list *node = list->next;

	while (nth) {
		--nth;
		node = node->next;
	}

	return node;
}

static struct wl_list *wl_list_remove_last(struct wl_list *list)
{
	struct wl_list *node = list->prev;

	wl_list_del(node);
	return node;
}

static void wl_list_move2_first(struct wl_list *list, struct wl_list *node)
{
	wl_list_link_node(node->prev, node->next);
	wl_list_insert_after(list, node);
}

#endif /* LIST_H_*/
