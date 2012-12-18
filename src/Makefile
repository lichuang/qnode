
CC=gcc
CFLAGS=-g -Wall -Werror

BINARY=qserver

ALL_O=main.o qactor.o qatomic.o qconfig.o qepoll.o qengine.o qhash.o qluacapi.o qluautil.o qmailbox.o qmalloc.o qmsg.o qnet.o \
			qserver.o qstring.o qthread.o qlog.o qpipe.o

all:$(ALL_O) $(BINARY)

ALL_O:
		$(CC) $< -o $@ -c $(CFLAGS)

$(BINARY):$(ALL_O)
		$(CC) $(ALL_O) -o $(BINARY) $(CFLAGS) -lpthread -rdynamic -llua -ldl -lm

clean:
		rm $(BINARY) $(ALL_O)

main.o: main.c qserver.h qserver.c
qactor.o:qactor.c qactor.h
qatomic.o:qatomic.c qatomic.h
qconfig.o:qconfig.c qconfig.h
qepoll.o: qepoll.c
qengine.o: qengine.c qengine.h qepoll.c qmalloc.h qmalloc.c
qhash.o: qhash.c qhash.h
qlog.o: qlog.h qlog.c
qluacapi.o: qluacapi.h qluacapi.c
qluautil.o: qluautil.h qluautil.c
qmailbox.o: qmailbox.h qmailbox.c qlist.h
qmalloc.o: qmalloc.h qmalloc.c
qmsg.o: qmsg.h qmsg.c
qnet.o: qnet.h qnet.c
qpipe.o: qpipe.h qpipe.c qatomic.h qatomic.c qmalloc.h qmalloc.c
qserver.o: qserver.h qserver.c
qstring.o: qstring.h qstring.c
qthread.o: qthread.h qthread.c
