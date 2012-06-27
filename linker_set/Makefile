CFLAGS=-O2 -Wall -g

.PHONY: all runtests

all: runtests

linker_set_test: linker_set_test.o set1.o set2.o
	cc -o linker_set_test linker_set_test.o set1.o set2.o

linker_set_test.o set1.o set2.o: linker_set.h

runtests: linker_set_test
	./linker_set_test

