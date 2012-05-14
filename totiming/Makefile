#CFLAGS=-O2 -Wall
CFLAGS=-g -Wall
CPPFLAGS=-DTEST_HARNESS -DDEBUG -I../avl -I../heap
VPATH=../avl ../heap

.PHONY: all runtests

all:: runtests

totest: totest.o kern_timeout.o

totest-avl: totest.o kern_timeout_avl.o subr_avl.o
	cc -o totest-avl totest.o kern_timeout_avl.o subr_avl.o

totest-heap:totest.o kern_timeout_heap.o heap.h
	cc -o totest-heap totest.o kern_timeout_heap.o

test.out: totest
	./totest | tee test.out

test-avl.out: totest-avl
	./totest-avl | tee test-avl.out

test-heap.out: totest-heap
	./totest-heap | tee test-heap.out

runtests:: test-heap.out test-avl.out test.out

