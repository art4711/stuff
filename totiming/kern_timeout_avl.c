/*	$OpenBSD: kern_timeout.c,v 1.33 2011/05/10 00:58:42 dlg Exp $	*/
/*
 * Copyright (c) 2001 Thomas Nordin <nordin@openbsd.org>
 * Copyright (c) 2000-2001 Artur Grabowski <art@openbsd.org>
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 *
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifdef TEST_HARNESS
#include <sys/time.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include "timeout.h"
#else
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/lock.h>
#include <sys/timeout.h>
#include <sys/mutex.h>
#include <sys/kernel.h>
#include <sys/queue.h>			/* _Q_INVALIDATE */
#endif

#ifdef DDB
#include <machine/db_machdep.h>
#include <ddb/db_interface.h>
#include <ddb/db_access.h>
#include <ddb/db_sym.h>
#include <ddb/db_output.h>
#endif

/*
 * All wheels are locked with the same mutex.
 *
 * We need locking since the timeouts are manipulated from hardclock that's
 * not behind the big lock.
 */
#ifdef TEST_HARNESS
#define mtx_enter(m)
#define mtx_leave(m)
int hz = 100;
int tick;
#define nitems(_a)	(sizeof((_a)) / sizeof((_a)[0]))
#define _Q_INVALIDATE(a) (a) = ((void *)-1)
#else
struct mutex timeout_mutex = MUTEX_INITIALIZER(IPL_HIGH);
#endif

struct avl_node *to_tree;

/*
 * Some of the "math" in here is a bit tricky.
 *
 * We have to beware of wrapping ints.
 * We use the fact that any element added to the queue must be added with a
 * positive time. That means that any element `to' on the queue cannot be
 * scheduled to timeout further in time than INT_MAX, but to->to_time can
 * be positive or negative so comparing it with anything is dangerous.
 * The only way we can use the to->to_time value in any predictable way
 * is when we calculate how far in the future `to' will timeout -
 * "to->to_time - ticks". The result will always be positive for future
 * timeouts and 0 or negative for due timeouts.
 */
extern int ticks;		/* XXX - move to sys/X.h */

void
timeout_startup(void)
{
}

int
t_cmp(const struct avl_node *an, const struct avl_node *bn)
{
	const struct timeout *a = avl_data(an, struct timeout, to_tree);
	const struct timeout *b = avl_data(bn, struct timeout, to_tree);

	if (a->to_time == b->to_time)
		return (intptr_t)a - (intptr_t)b;
	return a->to_time - b->to_time;
}

void
timeout_set(struct timeout *new, void (*fn)(void *), void *arg)
{
	new->to_func = fn;
	new->to_arg = arg;
	new->to_flags = TIMEOUT_INITIALIZED;
}

static int _timeout_del(struct timeout *);

void
timeout_add(struct timeout *new, int to_ticks)
{
#ifdef DIAGNOSTIC
	if (!(new->to_flags & TIMEOUT_INITIALIZED))
		panic("timeout_add: not initialized");
	if (to_ticks < 0)
		panic("timeout_add: to_ticks (%d) < 0", to_ticks);
#endif

	mtx_enter(&timeout_mutex);
	if (new->to_flags & TIMEOUT_ONQUEUE)
		_timeout_del(new);
	new->to_time = to_ticks + ticks;
	new->to_flags &= ~TIMEOUT_TRIGGERED;
	new->to_flags |= TIMEOUT_ONQUEUE;
	avl_insert(&new->to_tree, &to_tree, t_cmp);
	mtx_leave(&timeout_mutex);
}

void
timeout_add_tv(struct timeout *to, const struct timeval *tv)
{
	long long to_ticks;

	to_ticks = (long long)hz * tv->tv_sec + tv->tv_usec / tick;
	if (to_ticks > INT_MAX)
		to_ticks = INT_MAX;

	timeout_add(to, (int)to_ticks);
}

void
timeout_add_ts(struct timeout *to, const struct timespec *ts)
{
	long long to_ticks;

	to_ticks = (long long)hz * ts->tv_sec + ts->tv_nsec / (tick * 1000);
	if (to_ticks > INT_MAX)
		to_ticks = INT_MAX;

	timeout_add(to, (int)to_ticks);
}

#ifndef TEST_HARNESS
void
timeout_add_bt(struct timeout *to, const struct bintime *bt)
{
	long long to_ticks;

	to_ticks = (long long)hz * bt->sec + (long)(((uint64_t)1000000 *
	    (uint32_t)(bt->frac >> 32)) >> 32) / tick;
	if (to_ticks > INT_MAX)
		to_ticks = INT_MAX;

	timeout_add(to, (int)to_ticks);
}
#endif

void
timeout_add_sec(struct timeout *to, int secs)
{
	long long to_ticks;

	to_ticks = (long long)hz * secs;
	if (to_ticks > INT_MAX)
		to_ticks = INT_MAX;

	timeout_add(to, (int)to_ticks);
}

void
timeout_add_msec(struct timeout *to, int msecs)
{
	long long to_ticks;

	to_ticks = (long long)msecs * 1000 / tick;
	if (to_ticks > INT_MAX)
		to_ticks = INT_MAX;

	timeout_add(to, (int)to_ticks);
}

void
timeout_add_usec(struct timeout *to, int usecs)
{
	int to_ticks = usecs / tick;

	timeout_add(to, to_ticks);
}

void
timeout_add_nsec(struct timeout *to, int nsecs)
{
	int to_ticks = nsecs / (tick * 1000);

	timeout_add(to, to_ticks);
}

static int
_timeout_del(struct timeout *to)
{
	int ret = 0;
	if (to->to_flags & TIMEOUT_ONQUEUE) {
		avl_delete(&to->to_tree, &to_tree, t_cmp);
		to->to_flags &= ~TIMEOUT_ONQUEUE;
		ret = 1;
	}
	to->to_flags &= ~TIMEOUT_TRIGGERED;
	return ret;
}

int
timeout_del(struct timeout *to)
{
	int ret;

	mtx_enter(&timeout_mutex);
	ret = _timeout_del(to);
	mtx_leave(&timeout_mutex);

	return ret;
}

/*
 * This is called from hardclock() once every tick.
 * We return !0 if we need to schedule a softclock.
 */
int
timeout_hardclock_update(void)
{
	mtx_enter(&timeout_mutex);
	ticks++;
	mtx_leave(&timeout_mutex);

	return (1);
}

/*
 * Special compare for iterator.
 *
 * The 'b' node is always our iterator markers.
 */
static int
t_iter_cmp(struct avl_node *an, struct avl_node *bn)
{
	struct timeout *a = avl_data(an, struct timeout, to_tree);
	struct timeout *b = avl_data(bn, struct timeout, to_tree);

	if (a->to_time == b->to_time)
		return (intptr_t)a;
	return a->to_time - b->to_time;
}


void
softclock(void *arg)
{
	struct avl_node *n;
	struct timeout *to;
	struct timeout s;
	void (*fn)(void *);

	s.to_time = -1;

	mtx_enter(&timeout_mutex);
	while ((n = avl_search(&s.to_tree, &to_tree, t_cmp)) && (to = avl_data(n, struct timeout, to_tree))->to_time - ticks <= 0) {
		_timeout_del(to);
#ifdef DEBUG
		if (to->to_time - ticks < 0)
			printf("timeout delayed %d\n", to->to_time -
			    ticks);
#endif
		to->to_flags &= ~TIMEOUT_ONQUEUE;
		to->to_flags |= TIMEOUT_TRIGGERED;

		fn = to->to_func;
		arg = to->to_arg;

		mtx_leave(&timeout_mutex);
		fn(arg);
		mtx_enter(&timeout_mutex);
	}
	mtx_leave(&timeout_mutex);
}

#ifdef DDB
void db_show_callout_bucket(struct circq *);

void
db_show_callout_bucket(struct circq *bucket)
{
	struct timeout *to;
	struct circq *p;
	db_expr_t offset;
	char *name;

	for (p = CIRCQ_FIRST(bucket); p != bucket; p = CIRCQ_FIRST(p)) {
		to = (struct timeout *)p; /* XXX */
		db_find_sym_and_offset((db_addr_t)to->to_func, &name, &offset);
		name = name ? name : "?";
		db_printf("%9d %2d/%-4d %8x  %s\n", to->to_time - ticks,
		    (bucket - timeout_wheel) / WHEELSIZE,
		    bucket - timeout_wheel, to->to_arg, name);
	}
}

void
db_show_callout(db_expr_t addr, int haddr, db_expr_t count, char *modif)
{
	int b;

	db_printf("ticks now: %d\n", ticks);
	db_printf("    ticks  wheel       arg  func\n");

	db_show_callout_bucket(&timeout_todo);
	for (b = 0; b < nitems(timeout_wheel); b++)
		db_show_callout_bucket(&timeout_wheel[b]);
}
#endif
