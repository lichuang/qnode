/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QMSG_H__
#define __QMSG_H__

#include "qlist.h"

/* define messages between worker-thread and main-thread */
typedef struct qnode_msg_t {
  qnode_list_t entry;
} qnode_msg_t;

qnode_msg_t* qnode_msg_new();

#endif  /* __QMSG_H__ */
