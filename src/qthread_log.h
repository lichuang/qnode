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
  int             idx;
};

qthread_log_t*  qthread_log_init(int idx);
void            qthread_log_free();
qlog_t*         qthread_log_get();

#endif  /* __QTHREAD_LOG_H__ */
