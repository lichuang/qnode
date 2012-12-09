
CC=gcc
CFLAGS=-g -O -Wall -Werror

BINARY=qserver

ALL_O=main.o qatomic.o qepoll.o qengine.o qmailbox.o qmalloc.o qnet.o qserver.o qthread.o \
      qlog.o qpipe.o

all:$(ALL_O) $(BINARY)

ALL_O:
		$(CC) $< -o $@ -c $(CFLAGS)

$(BINARY):$(ALL_O)
		$(CC) $(ALL_O) -o $(BINARY) $(CFLAGS)

clean:
		rm $(BINARY) $(ALL_O)

main.o: main.c qserver.h qserver.c
qatomic.o:qatomic.c qatomic.h
qepoll.o: qepoll.c
qengine.o: qengine.c qengine.h qepoll.c qmalloc.h qmalloc.c
qlog.o: qlog.h qlog.c
qmailbox.o: qmailbox.h qmailbox.c qlist.h
qmalloc.o: qmalloc.h qmalloc.c
qnet.o: qnet.h qnet.c
qpipe.o: qpipe.h qpipe.c qatomic.h qatomic.c qmalloc.h qmalloc.c
qserver.o: qserver.h qserver.c
qthread.o: qthread.h qthread.c
