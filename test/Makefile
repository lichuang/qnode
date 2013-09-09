TESTS = freelist_test
INCLUDE = -I../src
CC=gcc
LDFLAGS= -L ../lib -lpthread -rdynamic -llua -ldl -lm -lldb -lreadline
CFLAGS=-Wall -Werror -g -DDEBUG

OBJS = ../obj/qfreelist.o \
			 ../obj/qalloc.o   \
			 ../obj/qassert.o  


all:$(TESTS)
	#for t in $(TESTS); do echo "***** Running $$t"; ./$$t || exit 1; done

freelist_test:freelist_test.c ctest.c
	$(CC) freelist_test.c ctest.c $(OBJS) $(LDFLAGS) $(CFLAGS) -o freelist_test -I../src
