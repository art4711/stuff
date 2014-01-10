#include <stdio.h>
#include "linker_set.h"

int
main(int argc, char **argv)
{
	int **i, sum;
	LINKER_SET_DECLARE(tset, int);

	sum = 0;
	LINKER_SET_FOREACH(i, tset) {
		sum += **i;
	}

	printf("%d\n", sum);

	return sum != 50;
}
