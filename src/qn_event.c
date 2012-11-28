/*
 * See Copyright Notice in qnode.h
 */

#include "qn_assert.h"
#include "qn_epoll.h"
#include "qn_event.h"
#include "qn_malloc.h"

qn_dispatcher_t* qn_dispatcher_create(int size) {
  qn_assert(size > 0);
  qn_dispatcher_t *dispatcher = qn_alloc_type(qn_dispatcher_t);
  dispatcher->max_fd = 0;
  dispatcher->events = qn_alloc_array(qn_io_event_t, size);
  dispatcher->size = size;
  if (qn_epoll_init(dispatcher) < 0) {
    qn_free(dispatcher->events);
    qn_free(dispatcher);
  }
  return dispatcher;
}
