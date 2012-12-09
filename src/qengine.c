/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qassert.h"
#include "qengine.h"
#include "qlog.h"
#include "qmalloc.h"

#define QNODE_RETIRED_FD -1

extern const struct qnode_dispatcher_t epoll_dispatcher;

static void init_qnode_event(qnode_event_t *event) {
    event->fd = QNODE_RETIRED_FD;
    event->flags = 0;
    event->read = event->write = NULL;
    event->data = NULL;
}

qnode_engine_t* qnode_engine_new() {
    qnode_engine_t *engine = qnode_alloc_type(qnode_engine_t);
    engine->max_fd = 0;
    engine->dispatcher = &(epoll_dispatcher);
    if (engine->dispatcher->init(engine) < 0) {
        qnode_free(engine);
        return NULL;
    }
    engine->events = qnode_alloc_array(qnode_event_t, QNODE_MAX_EVENTS);
    engine->active_events = qnode_alloc_array(qnode_event_t, QNODE_MAX_EVENTS);
    int i = 0;
    for (i = 0; i < QNODE_MAX_EVENTS; ++i) {
        qnode_event_t *event = &(engine->events[i]);
        init_qnode_event(event);
        event = &(engine->active_events[i]);
        init_qnode_event(event);
    }
    return engine;
}

int qnode_engine_add_event(qnode_engine_t *engine, int fd, int flags, qnode_event_func_t *callback, void *data) {
    if (fd > QNODE_MAX_EVENTS) {
        qnode_error("extends max fd");
        return -1;
    }
    qnode_event_t *event = &(engine->events[fd]);
    if (engine->dispatcher->add(engine, fd, flags) < 0) {
        qnode_error("add event error");
        return -1;
    }
    event->fd = fd;
    event->flags |= flags;
    event->data = data;
    if (flags & QNODE_EVENT_READ) {
        event->read = callback;
    }
    if (flags & QNODE_EVENT_WRITE) {
        event->write = callback;
    }
    if (fd > engine->max_fd) {
        engine->max_fd = fd;
    }
    return 0;
}

int qnode_engine_del_event(qnode_engine_t* engine, int fd, int flags) {
    if (fd > QNODE_MAX_EVENTS) {
        qnode_error("extends max fd");
        return -1;
    }
    if (flags == QNODE_EVENT_NONE) {
        return -1;
    }
    qnode_event_t *event = &(engine->events[fd]);
    event->flags = event->flags & (~flags);
    if (fd == engine->max_fd && event->flags == QNODE_EVENT_NONE) {
        int i;
        for (i = engine->max_fd - 1; i > 0; --i) {
            if (engine->events[i].flags != QNODE_EVENT_NONE) {
                engine->max_fd = i;
                break;
            }
        }
    }
    engine->dispatcher->del(engine, fd, flags);
    return 0;
}

int qnode_engine_loop(qnode_engine_t* engine) {
    int done = 0;
    int num, i;
    struct timeval time;
    while (!done) {
        time.tv_sec = time.tv_usec = 0;
        num = engine->dispatcher->poll(engine, NULL);
        for (i = 0; i < num; i++) {
            qnode_event_t *event = &(engine->events[engine->active_events[i].fd]);
            int flags = engine->active_events[i].flags;
            int fd = engine->active_events[i].fd;
            int read = 0;

            if (event->flags & flags & QNODE_EVENT_READ) {
                read = 1;
                event->read(fd, flags, event->data);
            }
            if (event->flags & flags & QNODE_EVENT_WRITE) {
                if (!read || event->write != event->read) {
                    event->write(fd, flags, event->data);
                }
            }
        }
    }
    return 0;
}

void qnode_engine_destroy(qnode_engine_t *engine) {
}

int qnode_engine_add_timer(qnode_engine_t* engine, struct timeval *time, int flags) {
    return 0;
}

int qnode_engine_del_timer(qnode_engine_t* engine, int id) {
    return 0;
}
