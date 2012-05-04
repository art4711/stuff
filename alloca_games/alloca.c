/*
 * Copyright (c) 2009 Artur Grabowski <art@blahonga.org>
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

#include <stdlib.h>
#include <stdio.h>

void
one_alloca(int sz)
{
	char *foo = alloca(sz);
	printf("one_alloca: %p\n", foo);
}

void
one_vla(int sz)
{
	char foo[sz];
	printf("one_vla: %p\n", foo);
}

void
five_alloca(int sz)
{
	int i;

	for (i = 0; i < 5; i++) {
		char *foo = alloca(sz);
		printf("five_alloca(%d): %p\n", i, foo);
	}
}

void
five_vla(int sz)
{
	int i;

	for (i = 0; i < 5; i++) {
		char foo[sz];
		printf("five_vla(%d): %p\n", i, foo);
	}
}

void
five_mixed(int sz)
{
	int i;

	for (i = 0; i < 5; i++) {
		char foo[sz];
		char *bar = alloca(sz);
		printf("five_mixed(%d): %p %p\n", i, foo, bar);
	}
}

int
main(int argc, char **argv)
{
	one_alloca(32);
	one_alloca(32);
	one_vla(32);
	one_vla(32);
	five_alloca(32);
	five_vla(32);
	five_mixed(32);
}

