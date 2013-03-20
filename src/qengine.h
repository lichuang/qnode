/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QENGINE_H__
#define __QENGINE_H__

#include <stdint.h>
#include <sys/time.h>
#include "qcore.h"
#include "qtimer.h"

#define QMAX_EVENTS  (1024 * 10)

/* for events */
#define QEVENT_NONE    0
#define QEVENT_READ    1
#define QEVENT_WRITE   2

/* for timers */
#define QTIMER_NONE    0
#define QTIMER_ONCE    1
#define QTIMER_PERSIST 2

struct qengine_t;
struct qevent_t;
struct qdispatcher_t;

typedef void (qevent_func_t)(int fd, int flags, void *data);

typedef struct qevent_t {
  int fd;
  int flags;
  qevent_func_t *read;
  qevent_func_t *write;
  void *data;
} qevent_t;

typedef struct qdispatcher_t {
  const char *name;
  int  (*init)(struct qengine_t*);
  int  (*add)(struct qengine_t *engine, int fd, int flags);
  int  (*del)(struct qengine_t *engine, int fd, int flags);
  int  (*poll)(struct qengine_t *, uint32_t timeout_ms);
  void (*destroy)(struct qengine_t *);
} qdispatcher_t;

struct qengine_t {
  struct qevent_t     *events;
  struct qevent_t     *active_events;
  const qdispatcher_t *dispatcher;
  void                *data;
  int                 max_fd;
  qmem_pool_t         *pool;
  qtimer_manager_t    timer_mng;
  uint64_t            now;
  char time_buff[20];
};

qengine_t* qengine_new(qmem_pool_t *pool);
int qengine_loop(qengine_t* engine);
void qengine_destroy(qengine_t *engine);

int qengine_add_event(qengine_t* engine, int fd, int flags, qevent_func_t *callback, void *data);
int qengine_del_event(qengine_t* engine, int fd, int flags);

qid_t qengine_add_timer(qengine_t* engine, uint32_t timeout_ms, qtimer_func_t *func, int type, void *data);
int qengine_del_timer(qengine_t* engine, qid_t id);

#endif  /* __QENGINE_H__ */
