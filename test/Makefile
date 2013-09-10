INCLUDE = -I../src
CC=gcc
LDFLAGS= -L ../lib -lpthread -rdynamic -llua -ldl -lm -lldb -lreadline
CFLAGS=-Wall -Werror -g -DDEBUG

OBJS = ../obj/qfreelist.o \
			 ../obj/qalloc.o   \
			 ../obj/qassert.o  \
			 ../obj/qbuffer.o   

TESTS = freelist_test \
	buffer_test

all:$(TESTS)
	for t in $(TESTS); do echo "\n***** Running $$t"; ./$$t || exit 1; done

freelist_test:freelist_test.c ctest.c ../obj/qfreelist.o
	$(CC) freelist_test.c ctest.c $(OBJS) $(LDFLAGS) $(CFLAGS) -o freelist_test $(INCLUDE)

buffer_test:buffer_test.c ctest.c ../obj/qbuffer.o
	$(CC) buffer_test.c ctest.c $(OBJS) $(LDFLAGS) $(CFLAGS) -o buffer_test $(INCLUDE)
