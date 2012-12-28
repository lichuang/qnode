/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QTHREAD_LOG_H__
#define __QTHREAD_LOG_H__

#include "qlist.h"

struct qmailbox_t;

typedef struct qthread_log_t {
  struct qlist_t lists[2];
  struct qlist_t *write;
  struct qlist_t *read;
  struct qmailbox_t *box;       /* thread log mailbox */
} qthread_log_t;

#endif  /* __QTHREAD_LOG_H__ */
