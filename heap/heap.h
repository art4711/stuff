/*
 * Copyright (c) 2006 Artur Grabowski <art@blahonga.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef HEAP_H
#define HEAP_H

#define HEAP_HEAD(name, type) 						\
struct name {								\
	type *hh_root;							\
	long hh_num;							\
}

#define HEAP_ENTRY(type)						\
	struct {							\
		type *he_link[2];					\
	}

#define HEAP_ENTRY_INITIALIZER						\
	{ NULL, NULL }

#define HEAP_INIT(head)	do {						\
	(head)->hh_root = NULL;						\
	(head)->hh_num = 0;						\
} while (0)

#define HEAP_FIRST(head) ((head)->hh_root)
#define HEAP_LEFT(elm, field) ((elm)->field.he_link[0])
#define HEAP_RIGHT(elm, field) ((elm)->field.he_link[1])

#define HEAP_EMPTY(head) ((head)->hh_root == NULL)

#define HEAP_INSERT(name, head, item) name##_HEAP_INSERT(head, item)
#define HEAP_REMOVE_HEAD(name, head) name##_HEAP_REMOVE_HEAD(head)
#define HEAP_UPDATE_HEAD(name, head) name##_HEAP_UPDATE_HEAD(head)

#define HEAP_PROTOTYPE(name, type, field, cmp, funprefix)		\
funprefix void    name##_HEAP_linkin(type **pp, long n, type *e);	\
funprefix void	name##_HEAP_INSERT(struct name *, type *);		\
funprefix void	name##_HEAP_REMOVE_HEAD(struct name *);			\
funprefix void	name##_HEAP_UPDATE_HEAD(struct name *);

/*
 * The HEAP is organized as a binary tree, with all levels filled except the last.
 *
 * New elements are inserted at the first free position, then moved upwards
 * until the heap condition is satisfied. The positions in the tree are enumerated
 * like this:
 *  root is at position 1,
 *  left side of the tree is all positions that have the lowest bit set,
 *  right side of the tree is all positions that don't have the lowest bit set.
 * This is recursive and for each level of the tree we shift away the bit we just
 * examined.
 *                          1
 *                   /             \
 *                11                 10
 *             /     \             /     \
 *          111       110       101       100
 *          / \       / \       / \       / \
 *       1111 1110 1101 1100 1011 1010 1001 1000
 *
 * Insertion is done through the linkin function which recurses down to the
 * first free position in the heap, inserts the element at that position,
 * then on the way back up from the recursion swaps the elements traversed
 * until the heap condition is satisfied. The linkin functions returns 0
 * when its job is done and no more compares are necessary.
 *
 * Head update is done by swapping the head node with its largest child
 * until the heap condition is satisfied.
 *
 * Head removal is done by moving the last element of the heap into the head
 * and then as a head update.
 */

#define HEAP_GENERATE(name, type, field, cmp, funprefix)		\
									\
funprefix void	       						\
name##_HEAP_linkin(type **pp, long n, type *e)				\
{									\
	int c;								\
	if (n == 1) {							\
		*pp = e;						\
		return;						\
	}								\
\
	if (cmp(e, *pp) < 0) {\
		type *t = *pp;\
		e->field = t->field;\
		*pp = e;\
		e = t;\
		t->field.he_link[0] = t->field.he_link[1] = NULL; \
	}\
	c = n & 1;							\
	name##_HEAP_linkin(&((*pp)->field.he_link[!c]), n>>1, e);	\
}									\
									\
funprefix void								\
name##_HEAP_INSERT(struct name *head, type *el)				\
{									\
	el->field.he_link[0] = el->field.he_link[1] = NULL;	       	\
	name##_HEAP_linkin(&head->hh_root, ++head->hh_num, el);		\
}									\
									\
funprefix void								\
name##_HEAP_REMOVE_HEAD(struct name *head)				\
{									\
	type **pp, *el, *r;						\
	int n;								\
	for (n = head->hh_num, pp = &head->hh_root; n != 1; n >>= 1)	\
		pp = &(*pp)->field.he_link[!(n & 1)];			\
	el = *pp;							\
	r = head->hh_root;						\
	head->hh_root = el;						\
	*pp = NULL;							\
	el->field = r->field;						\
	head->hh_num--;							\
	name##_HEAP_UPDATE_HEAD(head);					\
}									\
									\
funprefix void								\
name##_HEAP_UPDATE_HEAD(struct name *head)				\
{									\
	type **pp;							\
									\
	if (head->hh_root == NULL)					\
		return;							\
									\
	for (pp = &head->hh_root; (*pp)->field.he_link[1] != NULL;) {	\
		type *link[2];						\
		int c;							\
		link[0] = (*pp)->field.he_link[0];			\
		link[1] = (*pp)->field.he_link[1];			\
		c = link[0] != NULL && cmp(link[0], link[1]) <= 0;	\
		if (cmp((*pp), link[!c]) <= 0)				\
			return;						\
		(*pp)->field = link[!c]->field;				\
		link[!c]->field.he_link[!c] = *pp;			\
		link[!c]->field.he_link[c] = link[c];			\
		*pp = link[!c];						\
		pp = &(*pp)->field.he_link[!c];				\
	}								\
}

#endif /*HEAP_H*/

