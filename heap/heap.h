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

/*
 * HEAP implements INSERT, REMOVE_HEAD, UPDATE_HEAD.
 * PHEAP implements INSERT, REMOVE, UPDATE
 */
#define HEAP_HEAD(name, type) 						\
struct name {								\
	type *hh_root;							\
	long hh_num;							\
}

#define HEAP_ENTRY(type)						\
	struct {							\
		type *he_link[2];					\
	}

#define HEAP_ENTRY_INITIALIZER { NULL, NULL }

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
funprefix void								\
name##_HEAP_INSERT(struct name __restrict *head, type __restrict *el)	\
{									\
	type **lp;							\
	int n;								\
									\
	lp = &head->hh_root;						\
	for (n = ++head->hh_num; n > 1; n >>= 1) {			\
		if (cmp(el, *lp) < 0) {					\
			type *t = *lp;					\
			el->field = t->field;				\
			*lp = el;					\
			el = t;						\
		}							\
		lp = &(*lp)->field.he_link[!(n & 1)];			\
	}								\
	*lp = el;							\
	el->field.he_link[0] = el->field.he_link[1] = NULL;	       	\
}									\
									\
funprefix void								\
name##_HEAP_REMOVE_HEAD(struct name *head)				\
{									\
	type **lp, *el, *r;						\
	int n;								\
	for (n = head->hh_num, lp = &head->hh_root; n != 1; n >>= 1)	\
		lp = &(*lp)->field.he_link[!(n & 1)];			\
	el = *lp;							\
	r = head->hh_root;						\
	head->hh_root = el;						\
	*lp = NULL;							\
	el->field = r->field;						\
	head->hh_num--;							\
	name##_HEAP_UPDATE_HEAD(head);					\
}									\
									\
funprefix void								\
name##_HEAP_UPDATE_HEAD(struct name *head)				\
{									\
	type **lp;							\
									\
	if (head->hh_root == NULL)					\
		return;							\
									\
	for (lp = &head->hh_root; (*lp)->field.he_link[1] != NULL;) {	\
		type *link[2];						\
		int c;							\
		link[0] = (*lp)->field.he_link[0];			\
		link[1] = (*lp)->field.he_link[1];			\
		c = link[0] != NULL && cmp(link[0], link[1]) <= 0;	\
		if (cmp((*lp), link[!c]) <= 0)				\
			return;						\
		(*lp)->field = link[!c]->field;				\
		link[!c]->field.he_link[!c] = *lp;			\
		link[!c]->field.he_link[c] = link[c];			\
		*lp = link[!c];						\
		lp = &(*lp)->field.he_link[!c];				\
	}								\
}

#define PHEAP_HEAD(name, type) HEAP_HEAD(name, type)
#define PHEAP_ENTRY(type)						\
	struct {							\
		type *he_link[2];					\
		type *he_parent;					\
	}
#define PHEAP_ENTRY_INITIALIZER { NULL, NULL, NULL }
#define PHEAP_INIT(head) HEAP_INIT(head)
#define PHEAP_FIRST(head) HEAP_FIRST(head)
#define PHEAP_LEFT(el, field) HEAP_LEFT(el, field)
#define PHEAP_RIGHT(el, field) HEAP_RIGHT(el, field)
#define PHEAP_EMPTY(head) HEAP_EMPTY(head)
#define PHEAP_INSERT(name, head, item) name##_HEAP_INSERT(head, item)
#define PHEAP_REMOVE(name, head, item) name##_HEAP_REMOVE(head, item)
#define PHEAP_UPDATE(name, head, item) name##_HEAP_UPDATE(head, item)
#define PHEAP_REMOVE_HEAD(name, head) name##_HEAP_REMOVE(head, PHEAP_FIRST(head))
#define PHEAP_UPDATE_HEAD(name, head) name##_HEAP_UPDATE(head, PHEAP_FIRST(head))

#define PHEAP_PROTOTYPE(name, type, field, cmp, funprefix)		\
funprefix void name##_HEAP_INSERT(struct name *, type *);		\
funprefix void name##_HEAP_REMOVE(struct name *, type *);		\
funprefix void name##_HEAP_UPDATE(struct name *, type *);

#include <assert.h>

#define PHEAP_GENERATE(name, type, field, cmp, funprefix)		\
funprefix void								\
name##_HEAP_INSERT(struct name *head, type *el)				\
{									\
	type **lp, *parent = NULL;					\
	unsigned int n;							\
									\
	lp = &head->hh_root;						\
	for (n = ++head->hh_num; n > 1; n >>= 1) {			\
		parent = *lp;						\
		lp = &(*lp)->field.he_link[n & 1];			\
	}								\
	*lp = el;							\
	el->field.he_link[0] = el->field.he_link[1] = NULL;	       	\
	el->field.he_parent = parent;					\
	name##_HEAP_UPDATE(head, el);					\
}									\
funprefix void								\
name##_HEAP_REMOVE(struct name *head, type *el)				\
{									\
	type **lp, *last, *parent;					\
	unsigned int n;							\
	for (n = head->hh_num, lp = &head->hh_root; n > 1; n >>= 1)	\
		lp = &(*lp)->field.he_link[n & 1];			\
	head->hh_num--;							\
	last = *lp;							\
	*lp = NULL;							\
	if (last == el)							\
		return;							\
	if ((parent = el->field.he_parent) == NULL)			\
		head->hh_root = last;					\
	else								\
		parent->field.he_link[parent->field.he_link[1] == el] = last;	\
	last->field.he_parent = el->field.he_parent;\
	if ((last->field.he_link[0] = el->field.he_link[0]))		\
		last->field.he_link[0]->field.he_parent = last;		\
	if ((last->field.he_link[1] = el->field.he_link[1]))		\
		last->field.he_link[1]->field.he_parent = last;		\
	name##_HEAP_UPDATE(head, last);					\
}									\
									\
/*									\
 * Swaps one elements with its parent.					\
 */									\
funprefix void								\
name##_HEAP_SWAP(struct name *head, type *el)				\
{									\
	type *parent = el->field.he_parent;				\
	unsigned int el_link = parent->field.he_link[1] == el;		\
	type *pf, *pp;							\
									\
	if ((pp = parent->field.he_parent) == NULL)			\
		head->hh_root = el;					\
	else								\
		pp->field.he_link[pp->field.he_link[1] == parent] = el;	\
									\
	el->field.he_parent = pp;					\
	parent->field.he_parent = el;					\
	pf = parent->field.he_link[!el_link];				\
	if ((parent->field.he_link[0] = el->field.he_link[0]))		\
		parent->field.he_link[0]->field.he_parent = parent;	\
	if ((parent->field.he_link[1] = el->field.he_link[1]))		\
		parent->field.he_link[1]->field.he_parent = parent;	\
	el->field.he_link[el_link] = parent;				\
	el->field.he_link[!el_link] = pf;				\
	if (pf != NULL)							\
		pf->field.he_parent = el;				\
}									\
									\
funprefix void								\
name##_HEAP_UPDATE(struct name *head, type *el)				\
{									\
	type *parent;							\
	/*								\
	 * First see if it needs to go up.				\
	 */								\
	while ((parent = el->field.he_parent) && cmp(el, parent) < 0) {	\
		name##_HEAP_SWAP(head, el);				\
	}								\
	/*								\
	 * Now look if we need to push it down.				\
	 *								\
	 * Even when the element has been propagated up, it can		\
	 * still break the heap invariant on the last element.		\
	 *								\
	 * 0 gets filled in before 1					\
	 */								\
	while (el->field.he_link[0] != NULL) {				\
		int lower;						\
		lower = el->field.he_link[1] != NULL &&			\
		    cmp(el->field.he_link[0], el->field.he_link[1]) > 0;\
		if (cmp(el, el->field.he_link[lower]) <= 0)		\
			break;						\
		name##_HEAP_SWAP(head,el->field.he_link[lower]);	\
	}								\
}

#define CHEAP_HEAD(name, type) HEAP_HEAD(name, type)
#define CHEAP_ENTRY(type)						\
	struct {							\
		type *he_link[2];					\
		unsigned long he_pos;					\
	}
#define CHEAP_ENTRY_INITIALIZER { NULL, NULL, NULL }
#define CHEAP_INIT(head) HEAP_INIT(head)
#define CHEAP_FIRST(head) HEAP_FIRST(head)
#define CHEAP_LEFT(el, field) HEAP_LEFT(el, field)
#define CHEAP_RIGHT(el, field) HEAP_RIGHT(el, field)
#define CHEAP_EMPTY(head) HEAP_EMPTY(head)
#define CHEAP_INSERT(name, head, item) name##_HEAP_INSERT(head, item)
#define CHEAP_REMOVE(name, head, item) name##_HEAP_REMOVE(head, item)
#define CHEAP_UPDATE(name, head, item) name##_HEAP_UPDATE(head, item, item)
#define CHEAP_REMOVE_HEAD(name, head) name##_HEAP_REMOVE(head, CHEAP_FIRST(head))
#define CHEAP_UPDATE_HEAD(name, head) name##_HEAP_UPDATE(head, CHEAP_FIRST(head), CHEAP_FIRST(head))

#define CHEAP_PROTOTYPE(name, type, field, cmp, funprefix)		\
funprefix void name##_HEAP_INSERT(struct name *, type *);		\
funprefix void name##_HEAP_REMOVE(struct name *, type *);		\
funprefix void name##_HEAP_UPDATE(struct name *, type *, type*);

#define CHEAP_GENERATE(name, type, field, cmp, funprefix)		\
funprefix void								\
name##_HEAP_INSERT(struct name __restrict *head, type __restrict *el)	\
{									\
	type **lp;							\
	int n;								\
									\
	/*								\
	 * Walk down to the position of the last element, replace	\
	 * elements in our way with the new element if it's smaller.	\
	 * Since any element in a heap can be replaced with a smaller	\
	 * element as long as it's bigger or equal to the parent and	\
	 * still retain the heap invariant.				\
	 */								\
	lp = &head->hh_root;						\
	for (n = ++head->hh_num; n > 1; n >>= 1) {			\
		if (cmp(el, *lp) < 0) {					\
			type *t = *lp;					\
			el->field = t->field;				\
			*lp = el;					\
			el = t;						\
		}							\
		lp = &(*lp)->field.he_link[n & 1];			\
	}								\
	*lp = el;							\
	el->field.he_link[0] = el->field.he_link[1] = NULL;	       	\
	el->field.he_pos = head->hh_num;				\
}									\
funprefix void								\
name##_HEAP_REMOVE(struct name *head, type *el)				\
{									\
	type **lp, *rep;						\
	unsigned int n;							\
	for (n = head->hh_num, lp = &head->hh_root; n > 1; n >>= 1)	\
		lp = &(*lp)->field.he_link[n & 1];			\
	head->hh_num--;							\
	rep = *lp;							\
	*lp = NULL;							\
	if (rep == el)							\
		return;							\
	name##_HEAP_UPDATE(head, el, rep);				\
}									\
									\
/*									\
 * Update the element in position pos->he_pos with the element el.	\
 */									\
funprefix void								\
name##_HEAP_UPDATE(struct name *head, type *pos, type *el)		\
{									\
	type **lp;							\
	type *l0, *l1;							\
	unsigned long p = pos->field.he_pos;				\
	unsigned long n;						\
	l0 = pos->field.he_link[0];					\
	l1 = pos->field.he_link[1];					\
	lp = &head->hh_root;						\
	for (n = p; n > 1; n >>= 1) {					\
		if (cmp(el, *lp) < 0) {					\
			type *t = *lp;					\
			el->field = t->field;				\
			*lp = el;					\
			el = t;						\
		}							\
		lp = &(*lp)->field.he_link[n & 1];			\
	}								\
	el->field.he_link[0] = l0;					\
	el->field.he_link[1] = l1;					\
	el->field.he_pos = p;						\
	*lp = el;							\
	/*								\
	 * Now look if we need to push it down.				\
	 *								\
	 * Even when the element has been propagated up, it can		\
	 * still break the heap invariant on the last element.		\
	 *								\
	 * 0 gets filled in before 1					\
	 */								\
	while ((*lp)->field.he_link[0] != NULL) {			\
		type *l;						\
		int lower;						\
		lower = el->field.he_link[1] != NULL &&			\
		    cmp(el->field.he_link[0], el->field.he_link[1]) > 0;\
		l = el->field.he_link[lower];				\
		if (cmp(el, l) <= 0)					\
			break;						\
		*lp = l;						\
		l0 = l->field.he_link[0];				\
		l1 = l->field.he_link[1];				\
		p = l->field.he_pos;					\
		l->field.he_link[lower] = el;				\
		l->field.he_link[!lower] = el->field.he_link[!lower];	\
		l->field.he_pos = el->field.he_pos;			\
		el->field.he_link[0] = l0;				\
		el->field.he_link[1] = l1;				\
		el->field.he_pos = p;					\
		lp = &l->field.he_link[lower];				\
	}								\
}


#endif /*HEAP_H*/

