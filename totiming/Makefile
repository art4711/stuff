CFLAGS=-O2 -Wall
CPPFLAGS=-DTEST_HARNESS

.PHONY: all runtests

all:: runtests

totest: totest.o kern_timeout.o

test.out: totest
	./totest | tee test.out

runtests:: test.out
