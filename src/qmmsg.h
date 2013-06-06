/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QMMSG_H__
#define __QMMSG_H__

#include "qactor.h"
#include "qmsg.h"

/* M_* means Main thread handle message */
enum {
  M_SIGNAL  = 0,
  QMMSG_NUM
};

typedef struct qmmsg_signal_t {
  qmsg_header_fields;

  int         signo;
} qmmsg_signal_t;

qmsg_t* qmmsg_signal_new(int signo, qid_t sender, qid_t recver);

#endif  /* __QMMSG_H__ */
