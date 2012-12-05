/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QENGINE_H__
#define __QENGINE_H__

#include <sys/time.h>
#define QNODE_MAX_EVENTS  (1024 * 10)

/* for events */
#define QNODE_EVENT_NONE    0
#define QNODE_EVENT_READ    1
#define QNODE_EVENT_WRITE   2

/* for timers */
#define QNODE_TIMER_NONE    0
#define QNODE_TIMER_ONCE    1
#define QNODE_TIMER_PERSIST 2

struct qnode_engine_t;
struct qnode_event_t;
struct qnode_dispatcher_t;

typedef void (qnode_event_func_t)(int fd, int flags, void *data);
typedef void (qnode_timer_func_t)(void *data);

typedef struct qnode_event_t {
    int fd;
    int flags;
    qnode_event_func_t *read;
    qnode_event_func_t *write;
    void *data;
} qnode_event_t;

typedef struct qnode_timer_t {
    int flags;
    qnode_timer_func_t *callback;
    int min_heap_idx;
    void *data;
} qnode_timer_t;

typedef struct qnode_dispatcher_t {
  const char *name;
  int (*init)(struct qnode_engine_t*);
  int (*add)(struct qnode_engine_t *engine, int fd, int flags);
  int (*del)(struct qnode_engine_t *engine, int fd, int flags);
  int (*poll)(struct qnode_engine_t *, struct timeval *time);
} qnode_dispatcher_t;

typedef struct qnode_engine_t {
    struct qnode_event_t *events;
    struct qnode_event_t *active_events;
    int max_fd;
    const struct qnode_dispatcher_t *dispatcher;
    void *data;
} qnode_engine_t;

qnode_engine_t* qnode_engine_new();
int qnode_engine_loop(qnode_engine_t* engine);
void qnode_engine_destroy(qnode_engine_t *engine);

int qnode_engine_add_event(qnode_engine_t* engine, int fd, int flags, qnode_event_func_t *callback, void *data);
int qnode_engine_del_event(qnode_engine_t* engine, int fd, int flags);

int qnode_engine_add_timer(qnode_engine_t* engine, struct timeval *time, int flags);
int qnode_engine_del_timer(qnode_engine_t* engine, int id);

#endif  /* __QENGINE_H__ */
