/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qassert.h"
#include "qevent.h"
#include "qmalloc.h"

extern const struct qnode_event_op_t epoll_ops;

qnode_dispatcher_t* qnode_dispatcher_create(int size) {
  qnode_assert(size > 0);
  qnode_dispatcher_t *dispatcher = qnode_alloc_type(qnode_dispatcher_t);
  dispatcher->op = &epoll_ops;
  dispatcher->data = dispatcher->op->init(dispatcher);
  INIT_LIST_HEAD(&(dispatcher->event_list));
  INIT_LIST_HEAD(&(dispatcher->active_list));
  qnode_min_heap_ctor(&(dispatcher->timeheap));
  return dispatcher;
}

void qnode_event_init(qnode_event_t *event, int fd, short events,
                      qnode_event_fun_t *callback, void *data) {
  event->fd = fd;
  event->callback = callback;
  event->data = data;
  event->events = events;
  INIT_LIST_HEAD(&(event->active_node));
  INIT_LIST_HEAD(&(event->event_node));
  qnode_min_heap_elem_init(event);
}

int qnode_event_add(qnode_event_t *event, struct timeval time, qnode_dispatcher_t *dispatcher) {
  event->dispatcher = dispatcher;
  qnode_event_op_t *ops = dispatcher->op;
  list_add_tail(&(event->event), &(dispatcher->event_list));
}

int qnode_dispatcher_run(qnode_dispatcher_t *dispatcher) {
    const struct qnode_event_op_t *op = dispatcher->op;
    void *base = dispatcher->;
    struct timeval tv;
    struct timeval *tv_p;
    int res, done;

    while (!done) {
        res = op->dispatch(dispatch, base, tv_p);

        if (res == -1) {
            return -1;
        }
    }

    return 0;
}
