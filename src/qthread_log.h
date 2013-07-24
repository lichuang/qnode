/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QTHREAD_LOG_H__
#define __QTHREAD_LOG_H__

#include "qcore.h"
#include "qlimits.h"
#include "qlist.h"

/* per-thread log struct */
struct qthread_log_t {
  struct qlist_t  lists[2];
  struct qlist_t *write;
  struct qlist_t *read;
  int             idx;
};

qthread_log_t*  qthread_log_init(int idx);

//extern qthread_log_t* thread_log[QMAX_WORKER];

#endif  /* __QTHREAD_LOG_H__ */
