/*
 * See Copyright Notice in qnode.h
 */
#ifndef __QN_EVENT_H__
#define __QN_EVENT_H__

#include "qn_epoll.h"

struct qn_dispatcher_t;

typedef void (qn_io_event_func_t)(struct qn_dispatcher_t *dispatacher, int fd, void *data, int mask);

typedef struct qn_io_event_t {
  int mask;
  qn_io_event_func_t *read;
  qn_io_event_func_t *write;
  void *data;
} qn_io_event_t;

typedef int (qn_io_event_add_t)(struct qn_dispatcher_t *dispatacher, int fd, void *data, int mask);

typedef struct qn_dispatcher_t {
  int max_fd;
  struct qn_io_event_t *events;
  int size;
  void *data;

  qn_io_event_add_t *add;
} qn_dispatcher_t;

qn_dispatcher_t* qn_dispatcher_create(int size);
int qn_dispatcher_run(qn_dispatcher_t *dispatacher);

#endif  /* __QN_EVENT_H__ */
