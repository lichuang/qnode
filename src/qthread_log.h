/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QTHREAD_LOG_H__
#define __QTHREAD_LOG_H__

#include "qlist.h"

struct qengint_t;
struct qlog_t;

/* per-thread log struct */
typedef struct qthread_log_t {
  struct qlist_t lists[2];
  struct qlist_t *write;
  struct qlist_t *read;
  struct qengine_t *engine;
} qthread_log_t;

int qthread_log_init(struct qengine_t* engine);
struct qlog_t* qthread_log_get();

#endif  /* __QTHREAD_LOG_H__ */
