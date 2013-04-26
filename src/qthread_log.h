/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QTHREAD_LOG_H__
#define __QTHREAD_LOG_H__

#include "qcore.h"
#include "qlist.h"

/* per-thread log struct */
struct qthread_log_t {
  struct qlist_t  lists[2];
  struct qlist_t *write;
  struct qlist_t *read;
  qengine_t      *engine;
  int             idx;
};

qthread_log_t*  qthread_log_init(qengine_t* engine, int idx);
qlog_t*         qthread_log_get();
void            qthread_log_fetch(qthread_log_t *log, qlist_t **list);

#endif  /* __QTHREAD_LOG_H__ */
