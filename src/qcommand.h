/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QCOMMAND_H__
#ifndef __QCOMMAND_H__

/* define commands between worker-thread and main-thread */
struct qnode_list_t;
typedef struct qnode_command_t {
  struct qnode_list_t entry;
} qnode_command_t;

qnode_command_t* qnode_command_new();

#endif  /* __QCOMMAND_H__ */
