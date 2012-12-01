/*
 * See Copyright Notice in qnode.h
 */
#ifndef __QEVENT_H__
#define __QEVENT_H__

#include "list.h"

struct qnode_dispatcher_t;
struct qnode_min_heap_t;

typedef void (*qnode_event_fun_t)(int, short, void *arg);

struct qnode_event_t;

typedef struct qnode_event_t {
  int fd;
  void *data;
  struct qnode_dispatcher_t *dispatcher;
  qnode_event_fun_t *callback;
  short events;
  int flags;
  struct list_head active;      /* for active list */
  struct list_head event;       /* for dispatcher event list */
  unsigned int heap_idx;        /* for timeout heap */
  struct timeval timeout;
} qnode_event_t;

struct qnode_event_op_t {
  const char *name;
  void *(*init)(struct qnode_dispatcher_t*);
  int (*add)(void *, struct qnode_event_t *);
  int (*del)(void *, struct qnode_event_t *);
  int (*poll)(struct qnode_dispatcher_t *, void *, struct timeval *);
};

typedef struct qnode_dispatcher_t {
  struct qnode_event_op_t *op;
  void *data;

  struct list_head active_list; /* for active events */
  struct list_head event_list; /* for manage events */
  struct qnode_min_heap_t timeheap;
} qnode_event_t;
} qnode_dispatcher_t;

qnode_dispatcher_t* qnode_dispatcher_create();

int qnode_dispatcher_run(qnode_dispatcher_t *disptacher);

void qnode_event_init(qnode_event_t *event, int fd, short events,
                      qnode_event_fun_t *callback, void *data);

int qnode_event_add(qnode_event_t *event, struct timeval time, qnode_dispatcher_t *dispatcher);

#endif  /* __QEVENT_H__ */
