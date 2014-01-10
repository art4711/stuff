/*
 * Copyright (c) 2014 Artur Grabowski <art@blahonga.org>
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
#include <sys/ptrace.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <err.h>

int
main(int argc, char **argv)
{
	char template[] = "/tmp/tmpXXXXXX";
	void *d;
	pid_t p;
	int fd;
	int r;

	if ((fd = mkstemp(template)) == -1) {
		err(1, "mkstemp");
	}

	remove(template);
	if ((d = mmap(NULL, getpagesize(), PROT_READ, MAP_SHARED|MAP_FILE, fd, 0)) == MAP_FAILED) {
		err(1, "mmap");
	}

	switch ((p = fork())) {
	case -1:
		err(1, "fork");
		break;
	case 0:
		while (1)
			pause();
		_exit(1);
		break;
	default:
		break;
	}
	if (ptrace(PT_ATTACH, p, NULL, 0) == -1) {
		err(1, "PT_ATTACH");
	}
	r = ptrace(PT_READ_I, p, d, 0);

	warn("ptrace: %d", r);

	kill(p, 9);
	wait(NULL);
	return 0;
}
