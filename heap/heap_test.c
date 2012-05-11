#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <assert.h>

#include "heap.h"

struct el {
	HEAP_ENTRY(el) link;
	int val;
};

int
el_cmp(const struct el *a, const struct el *b)
{
	return a->val - b->val;
}

HEAP_HEAD(h, el) heap_root;

HEAP_PROTOTYPE(h, el, link, el_cmp)

HEAP_GENERATE(h, el, link, el_cmp)


int
main(int argc, char **argv)
{
	struct el *elems;
	struct el *el;
	int lowest = 0;
	int nelem;
	int nops;
	int added;
	int i;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <nelem> <nops>\n", argv[0]);
		exit(1);
	}

	nelem = atoi(argv[1]);
	nops = atoi(argv[2]);

	if ((elems = calloc(nelem, sizeof(*elems))) == NULL)
		err(1, "calloc");

	added = 0;
	for (i = 0; i < nops && (added < nelem || HEAP_FIRST(&heap_root)); i++) {
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
				HEAP_INSERT(h, &heap_root, el);
			}
			break;
		case 6:
		case 7:
			if ((el = HEAP_FIRST(&heap_root)) != NULL) {
				el->val = lowest + arc4random_uniform(10);
				HEAP_UPDATE_HEAD(h, &heap_root);
			}
			break;
		case 8:
		case 9:
			if ((el = HEAP_FIRST(&heap_root)) != NULL) {
				assert(lowest <= el->val);
				lowest = el->val;
				HEAP_REMOVE_HEAD(h, &heap_root);
			}
			break;
		}
	}

	return 0;
}
