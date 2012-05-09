#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include <sys/time.h>
#include <sys/queue.h>
#include "timeout.h"


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

static int fired;
static int added;
static int deleted;

static void
to_fire(void *v)
{
	struct myto *to = v;

	fired++;

	/* 1/3 chance to get rescheduled. */
	if (arc4random_uniform(3) == 0) {
		timeout_add(&to->to, random_time());
	} else {
		LIST_REMOVE(to, list);
		LIST_INSERT_HEAD(&inactive, to, list);
	}
}

int ticks;

int
main(int argc, char **argv)
{
	int nevents, nto;
	int i;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <number of timeouts> <number of events>\n", argv[0]);
		exit(1);
	}

	nto = atoi(argv[1]);
	nevents = atoi(argv[2]);

	if ((timeouts = calloc(nto, sizeof(*timeouts))) == NULL)
		err(1, "calloc");

	timeout_startup();
	LIST_INIT(&active);
	LIST_INIT(&inactive);

	for (i = 0; i < nto; i++) {
		timeout_set(&timeouts[i].to, to_fire, &timeouts[i].to);
		LIST_INSERT_HEAD(&inactive, &timeouts[i], list);
	}

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
			if (timeout_hardclock_update())
				softclock(NULL);
			break;
		case 1:
		case 2:
		case 3:
			deleted++;
			if ((to = LIST_FIRST(&active)) != NULL) {
				LIST_REMOVE(to, list);
				LIST_INSERT_HEAD(&inactive, to, list);
				timeout_del(&to->to);
			}
			break;
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
			timeout_add(&to->to, random_time());
			break;
		}
		if (i % 100 == 99) {
			printf("(a/d/f): %d/%d/%d (%f/%f/%f)\n", added, deleted, fired,
			    (double)added/(double)i,
			    (double)deleted/(double)i,
			    (double)fired/(double)i);
		}
	}

	return 0;
}
