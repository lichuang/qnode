/*
 * See Copyright Notice in qnode.h
 */

#include <errno.h>
#include <string.h>
#include <sys/epoll.h>
#include "qengine.h"
#include "qlog.h"
#include "qmalloc.h"

static int epoll_init (qnode_engine_t *engine);
static int epoll_add  (struct qnode_engine_t *engine, int fd, int flags);
static int epoll_del  (struct qnode_engine_t *engine, int fd, int flags);
static int epoll_poll (qnode_engine_t *engine, struct timeval *time);

const struct qnode_dispatcher_t epoll_dispatcher = {
    "epoll",
    epoll_init,
    epoll_add,
    epoll_del,
    epoll_poll,
};

typedef struct epoll_t {
    int fd;
    struct epoll_event events[QNODE_MAX_EVENTS];
} epoll_t;

static int epoll_init(struct qnode_engine_t *engine) {
    epoll_t *epoll = qnode_alloc_type(epoll_t);
    if (epoll == NULL) {
        return -1;
    }

    epoll->fd = epoll_create(1024);
    if (epoll->fd == -1) {
        return -1;
    }
    engine->data = epoll;
    return 0;
}

static int epoll_add(struct qnode_engine_t *engine, int fd, int flags) {
    epoll_t *epoll = engine->data;
    struct epoll_event event;
    int events = 0;
    
    int op = engine->events[fd].flags == QNODE_EVENT_NONE ?  EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    flags |= engine->events[fd].flags;
    if (flags & QNODE_EVENT_READ) {
        events |= EPOLLIN;
    }
    if (flags & QNODE_EVENT_WRITE) {
        events |= EPOLLOUT;
    }

    event.events = events;
    event.data.fd = fd;
    if (epoll_ctl(epoll->fd, op, fd, &event) == -1) {
        return -1;
    }
    return 0;
}

static int epoll_del(struct qnode_engine_t *engine, int fd, int delflags) {
    epoll_t *epoll  = engine->data;
    struct epoll_event event;
    int flags = engine->events[fd].flags & (~delflags);
    int events = 0;

    if (flags & QNODE_EVENT_READ) {
        events |= EPOLLIN;
    }
    if (flags & QNODE_EVENT_WRITE) {
        events |= EPOLLOUT;
    }
    event.data.fd = fd;
    event.events = events;
    if (flags != QNODE_EVENT_NONE) {
        epoll_ctl(epoll->fd, EPOLL_CTL_MOD, fd, &event);
    } else {
        epoll_ctl(epoll->fd, EPOLL_CTL_DEL, fd, &event);
    }
    return 0;
}

static int epoll_poll (qnode_engine_t *engine, struct timeval *time) {
    epoll_t *epoll = engine->data;
    int num = 0;

    num = epoll_wait(epoll->fd, epoll->events, QNODE_MAX_EVENTS,
        time ? (time->tv_sec*1000 + time->tv_usec/1000) : -1);

    if (num > 0) {
        int i;
        for (i = 0; i < num; i++) {
            int flags = 0;
            struct epoll_event *event = epoll->events + i;

            if (event->events & EPOLLIN) {
                flags |= QNODE_EVENT_READ;
            }
            if (event->events & EPOLLOUT) {
                flags |= QNODE_EVENT_WRITE;
            }
            engine->active_events[i].fd = event->data.fd;
            engine->active_events[i].flags = flags;
        }
    }
    return num;
}
