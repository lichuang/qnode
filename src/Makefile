
CC=gcc
CFLAGS=-g -Wall

BINARY=qnserver

ALL_O=main.o qepoll.o qevent.o qmalloc.o qnet.o qserver.o

all:$(ALL_O) $(BINARY)

ALL_O:
		$(CC) $< -o $@ -c $(CFLAGS)

$(BINARY):$(ALL_O)
		$(CC) $(ALL_O) -o $(BINARY) $(CFLAGS)

clean:
		rm $(BINARY) $(ALL_O)

main.o: main.c qevent.h qevent.c qserver.h qserver.c
qepoll.o: qepoll.h qepoll.c
qevent.o: qevent.c qevent.h qepoll.h qepoll.c qmalloc.h qmalloc.c
qmalloc.o: qmalloc.h qmalloc.c
qnet.o: qnet.h qnet.c
qserver.o: qserver.h qserver.c qevent.h qevent.c 
