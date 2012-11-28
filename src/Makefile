
CC=gcc
CFLAGS=-g -Wall

BINARY=qnserver

ALL_O=main.o qn_epoll.o qn_event.o qn_malloc.o qn_net.o qn_server.o

all:$(ALL_O) $(BINARY)

ALL_O:
		$(CC) $< -o $@ -c $(CFLAGS)

$(BINARY):$(ALL_O)
		$(CC) $(ALL_O) -o $(BINARY) $(CFLAGS)

clean:
		rm $(BINARY) $(ALL_O)

main.o: main.c qn_event.h qn_event.c qn_server.h qn_server.c
qn_epoll.o: qn_epoll.h qn_epoll.c
qn_event.o: qn_event.c qn_event.h qn_epoll.h qn_epoll.c qn_malloc.h qn_malloc.c
qn_malloc.o: qn_malloc.h qn_malloc.c
qn_net.o: qn_net.h qn_net.c
qn_server.o: qn_server.h qn_server.c qn_event.h qn_event.c 
