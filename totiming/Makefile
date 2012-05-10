CFLAGS=-O2 -Wall
CPPFLAGS=-DTEST_HARNESS -DDEBUG -I../avl
VPATH=../avl

.PHONY: all runtests

all:: runtests

totest: totest.o kern_timeout.o

totest-avl: totest.o kern_timeout_avl.o subr_avl.o
	cc -o totest-avl totest.o kern_timeout_avl.o subr_avl.o

test.out: totest
	./totest | tee test.out

test-avl.out: totest-avl
	./totest-avl | tee test-avl.out

runtests:: test-avl.out test.out

