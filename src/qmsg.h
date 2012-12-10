/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QMSG_H__
#define __QMSG_H__

#include "qlist.h"

/* define messages between worker-thread and main-thread */
typedef struct qmsg_t {
  qlist_t entry;
} qmsg_t;

qmsg_t* qmsg_new();

#endif  /* __QMSG_H__ */
