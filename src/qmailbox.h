/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QMAILBOX_H__
#define __QMAILBOX_H__

#include "qlist.h"

struct qnode_command_t;
struct qnode_list_t;

typedef struct qnode_mailbox_t {
  struct qnode_list_t *list;
} qnode_mailbox_t;

qnode_mailbox_t* qnode_mailbox_new();
void             qnode_mailbox_add(qnode_mailbox_t *box, struct qnode_command_t *cmd);
void             qnode_mailbox_get(qnode_mailbox_t *box, struct qnode_list_t *list);

#endif  /* __QMAILBOX_H__ */
