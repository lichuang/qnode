/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qassert.h"
#include "qevent.h"
#include "qmalloc.h"
#include "qminheap.h"

extern const struct qnode_event_op_t epoll_ops;

qnode_dispatcher_t* qnode_dispatcher_create(int size) {
  qnode_assert(size > 0);
  qnode_dispatcher_t *dispatcher = qnode_alloc_type(qnode_dispatcher_t);
  dispatcher->op = &epoll_ops;
  dispatcher->data = dispatcher->op->init(dispatcher);
  INIT_LIST_HEAD(&(dispatcher->event_list));
  INIT_LIST_HEAD(&(dispatcher->active_list));
  qnode_minheap_ctor(&(dispatcher->timeheap));
  return dispatcher;
}

void qnode_event_init(qnode_event_t *event, int fd, short events,
                      qnode_event_fun_t *callback, void *data) {
  event->fd = fd;
  event->callback = callback;
  event->data = data;
  event->events = events;
  INIT_LIST_HEAD(&(event->active_entry));
  INIT_LIST_HEAD(&(event->event_entry));
  qnode_minheap_elem_init(event);
}

static void event_queue_insert(qnode_dispatcher_t *dispatcher, qnode_event_t *event, int queue) {
  if (event->flags & queue) {
    /* Double insertion is possible for active events */
    if (queue & QEVENT_LIST_ACTIVE)
      return;
  }

  if (~event->flags & QEVENT_LIST_INTERNAL) {
    base->event_count++;
  }
  event->flags |= queue;
  switch (queue) {
    case QEVENT_LIST_INSERTED:
      list_add_tail(&(event->event_entry), &(dispatcher->event_list));
      break;
    case QEVENT_LIST_ACTIVE:
      dispatcher->active_event_count++;
      list_add_tail(&(event->active_entry), &(dispatcher->active_list));
      break;
    case QEVENT_LIST_TIMEOUT:
      qnode_minheap_push(&(dispatcher->timeheap), event);
      break;
    default:
      break;
  }
}

int qnode_event_add(qnode_event_t *event, struct timeval time, qnode_dispatcher_t *dispatcher) {
  event->dispatcher = dispatcher;
  const qnode_event_op_t *ops = dispatcher->op;
  int result = 0;

  qassert(!(event->flags & ~EVENT_LIST_ALL));

  if (time != NULL && !(event->flags & QEVENT_LIST_TIMEOUT)) {
    if (qnode_min_heap_reserve(&(dispatcher->timeheap),
                               1 + qnode_minheap_size(&dispatcher->timeheap)) == -1) {
      return (-1);
    }
  }

  if ((event->events & (QEVENT_READ|QEVENT_WRITE|QEVENT_SIGNAL)) &&
      !(event->flags & (QEVENT_LIST_INSERTED|QEVENT_LIST_ACTIVE))) {
    result = ops->add(evbase, ev);
    if (result != -1) {
      event_queue_insert(dispatcher, ev, QEVENT_LIST_INSERTED);
    }
  }

  if (result != -1 && tv != NULL) {
    struct timeval now;

    if (ev->ev_flags & QEVENT_LIST_TIMEOUT)
      event_queue_remove(dispatcher, event, QEVENT_LIST_TIMEOUT);

    if ((event->flags & QEVENT_LIST_ACTIVE) && (event->result & EV_TIMEOUT)) {
      event_queue_remove(dispatcher, event, QEVENT_LIST_ACTIVE);
    }

    gettime(dispatcher, &now);
    evutil_timeradd(&now, tv, &ev->ev_timeout);

    event_queue_insert(dispatcher, ev, QEVENT_LIST_TIMEOUT);
  }

  return result;
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


void qnode_event_active(qnode_event_t *event, int result, short ncalls) {
  if (event->flags & QEVENT_LIST_ACTIVE) {
    event->result |= result;
    return;
  }

  event->result = result;
  event->ncalls = ncalls;
  event_queue_insert(event->dispatcher, event, QEVENT_LIST_ACTIVE);
}
