/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QCOMMAND_H__
#define __QCOMMAND_H__

#include "qlist.h"

/* define commands between worker-thread and main-thread */
typedef struct qnode_command_t {
  qnode_list_t entry;
} qnode_command_t;

qnode_command_t* qnode_command_new();

#endif  /* __QCOMMAND_H__ */
