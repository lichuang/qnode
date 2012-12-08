
CC=gcc
CFLAGS=-g -Wall -Werror

BINARY=qserver

ALL_O=main.o qepoll.o qengine.o qmalloc.o qnet.o qserver.o qlog.o \
	  qatomic.o qpipe.o

all:$(ALL_O) $(BINARY)

ALL_O:
		$(CC) $< -o $@ -c $(CFLAGS)

$(BINARY):$(ALL_O)
		$(CC) $(ALL_O) -o $(BINARY) $(CFLAGS)

clean:
		rm $(BINARY) $(ALL_O)

main.o: main.c qevent.h qevent.c qserver.h qserver.c
qatomic.o:qatomic.c qatomic.h
qepoll.o: qepoll.c
qengine.o: qengine.c qengine.h qepoll.c qmalloc.h qmalloc.c
qlog.o: qlog.h qlog.c
qmalloc.o: qmalloc.h qmalloc.c
qnet.o: qnet.h qnet.c
qpipe.o: qpipe.h qpipe.c qatomic.h qatomic.c qmalloc.h qmalloc.c
qserver.o: qserver.h qserver.c qevent.h qevent.c 
