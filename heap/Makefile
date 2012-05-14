CFLAGS=-O2 -Wall
#CFLAGS=-g -Wall
CPPFLAGS=-DTEST_HARNESS -DDEBUG -I../avl
VPATH=../avl

.PHONY: all runtests

all: runtests

heap_test: heap_test.o
	cc -o heap_test heap_test.o

heap_test.o: heap.h

test.out: heap_test
	./heap_test | tee test.out

runtests: test.out


