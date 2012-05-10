#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include <sys/time.h>
#include <sys/queue.h>
#include "timeout.h"

#ifdef __MACH__
#include <mach/mach_time.h>
#endif


/*
 * We only generate 0-100k. 100k is 1000 seconds.
 */
static int
random_time(void)
{
	/*
	 * 50% chance to end up between 0-49
	 * 25% chance to end up 50-9999
	 * 25% chance to end up 1000-100k
	 */
	switch (arc4random_uniform(4)) {
	case 0:
	case 1:
		return arc4random_uniform(50);
	case 2:
		return 50 + arc4random_uniform(1000 - 50);
	case 3:
		return 1000 + arc4random_uniform(100000 - 1000);
	}
	return 0;
}


LIST_HEAD(x,myto) active, inactive;

struct myto {
	struct timeout to;
	LIST_ENTRY(myto) list;
} *timeouts;

int fired;

static void
to_fire(void *v)
{
	struct myto *to = v;

	fired++;

	LIST_REMOVE(to, list);
	LIST_INSERT_HEAD(&inactive, to, list);
}

int ticks;

static uint64_t
timestamp(void)
{
	return mach_absolute_time();
}

static double
ts2ns(uint64_t ts)
{
	static mach_timebase_info_data_t tb;
	if (tb.denom == 0)
		mach_timebase_info(&tb);
	return ((double)ts * (double)tb.numer / (double)tb.denom);
}

void
run_one(int nto, int nevents) {
	uint64_t start, end;
	uint64_t fs, as, ds;
	uint64_t s;
	struct myto *to;
	double elapsed;
	int deleted;
	int added;
	int i;
	int rt;

	if ((timeouts = calloc(nto, sizeof(*timeouts))) == NULL)
		err(1, "calloc");

	LIST_INIT(&active);
	LIST_INIT(&inactive);

	for (i = 0; i < nto; i++) {
		timeout_set(&timeouts[i].to, to_fire, &timeouts[i].to);
		LIST_INSERT_HEAD(&inactive, &timeouts[i], list);
	}

	fs = as = ds = 0;

	added = deleted = fired = 0;

	start = timestamp();

	for (i = 0; i < nevents; i++) {
		struct myto *to;

		/*
		 * 60% chance to schedule some timeout.
		 * 30% chance to remove one.
		 * 10% chance to fire softclock.
		 */
		switch(arc4random_uniform(10)) {
		case 0:
			ticks++;
			fired++;
			s = timestamp();
			if (timeout_hardclock_update())
				softclock(NULL);
			fs += timestamp() - s;
			break;
		case 1:
		case 2:
			if ((to = LIST_FIRST(&active)) != NULL) {
				LIST_REMOVE(to, list);
				LIST_INSERT_HEAD(&inactive, to, list);
				deleted++;
				s = timestamp();
				timeout_del(&to->to);
				ds += timestamp() - s;
			}
			break;
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			added++;
			to = &timeouts[arc4random_uniform(nto)];
			LIST_REMOVE(to, list);
			LIST_INSERT_HEAD(&active, to, list);
			rt = random_time();
			s = timestamp();
			timeout_add(&to->to, rt);
			as += timestamp() - s;
			break;
		}
	}
	end = timestamp();


#define av(at,s) (ts2ns(at) / (double)s)
	elapsed = ts2ns(end - start);

	printf("%d %d %f %f %f %f\n", nto, nevents, elapsed / 1000000000.0, av(as, added), av(ds, deleted), av(fs, fired));
	fflush(stdout);

	LIST_FOREACH(to, &active, list) {
		timeout_del(&to->to);
	}
	free(timeouts);
}

#ifndef nitems
#define nitems(_a)	(sizeof((_a)) / sizeof((_a)[0]))
#endif
int test_tos[] = {
	10, 20, 50, 100, 200, 1000, 2000, 5000, 10000, 20000, 50000, 100000, 200000, 500000, 1000000
};

int
main(int argc, char **argv)
{
	int nevents, nto;
	int i;

	timeout_startup();

	if (argc == 3) {
		nto = atoi(argv[1]);
		nevents = atoi(argv[2]);
		run_one(nto, nevents);
		return 0;
	}

	for (i = 0; i < nitems(test_tos) - 1; i++) {
		int distance = test_tos[i + 1] - test_tos[i];
		int steps = distance > 20 ? 20 : distance;
		int j;
		for (j = 0; j < steps; j++) {
			run_one(((test_tos[i + 1] - test_tos[i]) / steps) * j + test_tos[i], 1000000);
		}
	}

	return 0;
}
