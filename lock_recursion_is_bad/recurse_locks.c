/*
 * Copyright (c) 2012 Artur Grabowski <art@blahonga.org>
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

#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
	char tname[32] = "/tmp/XXXXXXXXXXXXXXXXXXXXX";
	int fd;
	int ps = getpagesize();
	char *d;
	int i;

	if ((fd = mkstemp(tname)) == -1)
		err(1, "mkstemp");

	unlink(tname);
	if (ftruncate(fd, ps) == -1)
		err(1, "ftrunc");

	if ((d = mmap(NULL, ps, PROT_READ, MAP_SHARED|MAP_FILE, fd, 0)) == MAP_FAILED)
		err(1, "mmap");

	if (write(fd, d, ps) != ps)
		err(0, "write (expected)");

	warnx("write successful");

	for (i = 0; i < ps; i += 8) {
		int j;

		for (j = 0; j < 8; j++) {
			unsigned char c = d[i * 8 + j];
			printf("0x%x ", (unsigned int)c);
		}
		for (j = 0; j < 8; j++) {
			unsigned char c = d[i * 8 + j];
			printf("%c ", isprint(c) ? c : '.');
		}
	}

	return 1;
}
