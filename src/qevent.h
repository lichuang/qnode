/*
 * See Copyright Notice in qnode.h
 */
#ifndef __QEVENT_H__
#define __QEVENT_H__

#include <sys/time.h>
#include "list.h"
#include "qminheap.h"

#define QEVENT_LIST_TIMEOUT  0x01
#define QEVENT_LIST_INSERTED 0x02
#define QEVENT_LIST_SIGNAL   0x04
#define QEVENT_LIST_ACTIVE   0x08
#define QEVENT_LIST_INTERNAL 0x10
#define QEVENT_LIST_INIT     0x80

#define QEVENT_LIST_ALL      (0xf000 | 0x9f)

#define QEVENT_TIMEOUT      0x01
#define QEVENT_READ         0x02
#define QEVENT_WRITE        0x04
#define QEVENT_SIGNAL       0x08
#define QEVENT_PERSIST      0x10    /* Persistant event */

struct qnode_dispatcher_t;
struct qnode_event_t;

typedef int (*qnode_event_fun_t)(int fd, short flag, void *arg);

typedef struct qnode_event_t {
  int fd;
  void *data;
  struct qnode_dispatcher_t *dispatcher;
  qnode_event_fun_t *callback;
  short events;
  int flags;
  int results;
  struct list_head active_entry;      /* for active list */
  struct list_head event_entry;       /* for dispatcher event list */
  unsigned int heap_idx;        /* for timeout heap */
  struct timeval timeout;
} qnode_event_t;

typedef struct qnode_event_op_t {
  const char *name;
  void *(*init)(struct qnode_dispatcher_t*);
  int (*add)(void *, struct qnode_event_t *);
  int (*del)(void *, struct qnode_event_t *);
  int (*poll)(struct qnode_dispatcher_t *, void *, struct timeval *);
} qnode_event_op_t;

typedef struct qnode_dispatcher_t {
  const struct qnode_event_op_t *op;
  void *data;
  unsigned int event_count;
  unsigned int active_event_count;
  struct list_head active_list; /* for active events */
  struct list_head event_list; /* for manage events */
  struct qnode_minheap_t timeheap;
} qnode_dispatcher_t;

qnode_dispatcher_t* qnode_dispatcher_create();

int qnode_dispatcher_run(qnode_dispatcher_t *disptacher);

void qnode_event_init(qnode_event_t *event, int fd, short events,
                      qnode_event_fun_t *callback, void *data);

int qnode_event_add(qnode_event_t *event, struct timeval time, qnode_dispatcher_t *dispatcher);

void qnode_event_active(qnode_event_t *event, int result, short ncalls);

#endif  /* __QEVENT_H__ */
