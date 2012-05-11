CFLAGS=-O2 -Wall
CPPFLAGS=-DTEST_HARNESS -DDEBUG -I../avl
VPATH=../avl

.PHONY: all runtests

all: runtests

heap_test: heap_test.o
	cc -o heap_test heap_test.o

runtests: heap_test
	./heap_test 1000

