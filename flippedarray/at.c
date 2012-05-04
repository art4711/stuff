#include <stdlib.h>
#include <stdio.h>
#include "queue.h"

struct cache_line {
	TAILQ_ENTRY(cache_line) lru;
	int l;
};
TAILQ_HEAD(,cache_line) cache_lru = TAILQ_HEAD_INITIALIZER(cache_lru);

int cl_linesize;
int cl_linesize2;

int cl_hit;
int cl_miss;

static void
cache_init(int cl_size, int cl_linesz)
{
	int lines;
	struct cache_line *cl;
	int i;

	cl_linesize2 = cl_linesz;
	cl_linesize = 1 << cl_linesz;

	lines = cl_size / cl_linesize;
	cl = calloc(lines, sizeof(*cl));
	for (i = 0; i < lines; i++) {
		cl[i].l = -1;
		TAILQ_INSERT_TAIL(&cache_lru, &cl[i], lru);
	}
}

static void
cache_access(int addr)
{
	struct cache_line *cl;
	int l = addr / cl_linesize;

	for (cl = TAILQ_FIRST(&cache_lru); TAILQ_NEXT(cl, lru) != NULL; cl = TAILQ_NEXT(cl, lru))
		if (cl->l == l)
			break;
	if (cl->l == l)
		cl_hit++;
	else
		cl_miss++;
	cl->l = addr / cl_linesize;
	TAILQ_REMOVE(&cache_lru, cl, lru);
	TAILQ_INSERT_HEAD(&cache_lru, cl, lru);

#if 0
	printf("h/m: %d/%d = %f\n", cl_hit, cl_miss, (double)cl_hit/(double)cl_miss);
#endif
}

unsigned long long sum_jump;

int *
hs(int *haystack, int hs2, unsigned int index, int c)
{
	unsigned int i;
	static int *last_hs;
	int *ret;
#if 0
	i = index;
#elif 0
	int n = 0;
	i = 0;

	while (1) {
		if (index & (1 << n))
			i |= 1 << (hs2 - n - 1);
		if (++n == hs2)
			break;
	}
#else
	i = index;

	i = ((i >> 1) & 0x55555555) | ((i << 1) & 0xaaaaaaaa);
	i = ((i >> 2) & 0x33333333) | ((i << 2) & 0xcccccccc);
	i = ((i >> 4) & 0x0f0f0f0f) | ((i << 4) & 0xf0f0f0f0);
	i = ((i >> 8) & 0x00ff00ff) | ((i << 8) & 0xff00ff00);
	i = ((i >> 16) & 0x0000ffff) | ((i << 16) & 0xffff0000);
	i--;
	i >>= 32 - hs2;
#if 0
	/* Flip upper half of the bits, but not the lower */
	unsigned int mask = ((1 << (hs2 / 2)) - 1);
	i &= 0xffffffffU ^ mask;
	i |= index & mask;
#endif
#endif

	ret = &haystack[i];
	if (c) {
		cache_access(i * sizeof(*haystack));
		if (last_hs)
			sum_jump += abs(last_hs - ret);
		last_hs = ret;
	}
	return ret;
}

static int
lookup(int *haystack, int haystacksz, int hs2, int needle)
{
        int i, ns, no;
        int res;

        /*
         * Binary search in the del array.
         */
	no = 0;
	ns = haystacksz;

	while (ns > 0) {
                int even = ~ns & 1;

                ns /= 2;
	        i = no + ns;

                res = *hs(haystack, hs2, i, 1) - needle;
	        if (res == 0)
		        return 1;
                if (res > 0)
                        continue;
                no += ns + 1;
		ns -= even;
        }

	return 0;
}

int
main(int argc, char **argv)
{
	int *haystack;
	int nelem, rounds;
	int i, x, hs2;

	cache_init(65536, 7);

	if (argc != 3)
		errx(1, "usage: %s <nelem> <rounds>\n", argv[0]);

	nelem = atoi(argv[1]);
	rounds = atoi(argv[2]);

	for (i = 1; i < 8 * sizeof(int); i *= 2)
		nelem |= nelem >> i;
	nelem += 1;
	printf("size: 0x%x\n", nelem);


	for (i = nelem, hs2 = -1; i > 0; i >>= 1)
		hs2 += 1;

	printf("hs2: 0x%x : 0x%x\n", hs2, 1 << hs2);

	haystack = malloc(nelem * sizeof(*haystack));
	for (i = 0; i < nelem; i++) {
		*hs(haystack, hs2, i, 0) = i;
	}

#if 0
	for (i = 0; i < nelem; i++) {
		printf("0x%x\n", haystack[i]);
	}
#endif

	srand(time(NULL));

	cl_hit = cl_miss = 0;

	for (i = 0; i < rounds; i++) {
		if (!lookup(haystack, nelem, hs2, (x = (rand() % nelem))))
			errx(1, "not found %d %d\n", x, nelem);
		if (i < 10000) {
			printf("miss/search(%d): %f, avg jump: %f\n", i, (double)cl_miss/(double)i, (double)sum_jump/(double)i);
		} else {
			if (i == 10000)
				cl_miss = cl_hit = 0;
			else
				printf("FOOmiss/search: %f, avg jump: %f\n", (double)cl_miss/(double)(i - 10000), (double)sum_jump/(double)i);
		}
	}

	return 0;
}
