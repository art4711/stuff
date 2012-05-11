#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <assert.h>
#include <inttypes.h>

#include "heap.h"

#if 0
#include <sys/resource.h>
#else
#ifdef __MACH__
#include <mach/mach_time.h>
#endif
#endif

struct el {
	HEAP_ENTRY(el) link;
	int val;
};

static inline int
el_cmp(const struct el *a, const struct el *b)
{
	return a->val - b->val;
}

HEAP_HEAD(h, el) heap_root;

HEAP_PROTOTYPE(h, el, link, el_cmp)

HEAP_GENERATE(h, el, link, el_cmp)

static uint64_t
timestamp(void)
{
#if 0
	struct rusage ru;
	getrusage(RUSAGE_SELF, &ru);
	return ((uint64_t)ru.ru_utime.tv_sec * 1000000ULL) + (uint64_t)ru.ru_utime.tv_usec;
#else
	return mach_absolute_time();
#endif
}

static double
ts2ns(uint64_t ts)
{
#if 0
	return (double)ts * 1000.0;
#else
	static mach_timebase_info_data_t tb;
	if (tb.denom == 0)
		mach_timebase_info(&tb);
	return ((double)ts * (double)tb.numer / (double)tb.denom);
#endif
}

int
main(int argc, char **argv)
{
	struct el *elems;
	struct el *el;
	int lowest = 0;
	uint64_t s;
	uint64_t it, ut, rt;
	int inserts, updates, removes;
	int nelem;
	int added;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <nelem>\n", argv[0]);
		exit(1);
	}

	nelem = atoi(argv[1]);

	if ((elems = calloc(nelem, sizeof(*elems))) == NULL)
		err(1, "calloc");

	inserts = updates = removes = 0;
	it = ut = rt = 0;

	added = 0;
	while (added < nelem || HEAP_FIRST(&heap_root)) {
		switch (arc4random_uniform(10)) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			if (added < nelem) {
				el = &elems[added++];
				el->val = lowest + arc4random_uniform(nelem);
				s = timestamp();
				HEAP_INSERT(h, &heap_root, el);
				it += timestamp() - s;
				inserts++;
			}
			break;
		case 6:
		case 7:
			if ((el = HEAP_FIRST(&heap_root)) != NULL) {
				el->val = lowest + arc4random_uniform(10);
				s = timestamp();
				HEAP_UPDATE_HEAD(h, &heap_root);
				ut += timestamp() - s;
				updates++;
			}
			break;
		case 8:
		case 9:
			if ((el = HEAP_FIRST(&heap_root)) != NULL) {
				assert(lowest <= el->val);
				lowest = el->val;
				s = timestamp();
				HEAP_REMOVE_HEAD(h, &heap_root);
				rt += timestamp() - s;
				removes++;
			}
			break;
		}
	}

	printf("%d %f %f %f\n", nelem, ts2ns(it) / inserts, ts2ns(ut) / updates, ts2ns(rt) / removes);

	return 0;
}
