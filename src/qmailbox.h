/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QMAILBOX_H__
#define __QMAILBOX_H__

#include "qatomic.h"
#include "qengine.h"
#include "qlist.h"

struct qnode_msg_t;
struct signal_t;

typedef struct qnode_mailbox_t {
  struct qnode_list_t lists[2]; /* one for read, one for write */
  struct qnode_list_t *write;   /* current write list */
  struct qnode_list_t *read;    /* current write list */
  qnode_atomic_t active;        /* active if there is mail */
  qnode_event_func_t *callback; /* mailbox reader callback */
  void *reader;                 /* mailbox reader */
  struct signaler_t *signal;
} qnode_mailbox_t;

qnode_mailbox_t* qnode_mailbox_new(qnode_event_func_t *callback, void *reader);
void             qnode_mailbox_destroy(qnode_mailbox_t *box);
int              qnode_mailbox_active(qnode_engine_t *engine, qnode_mailbox_t *box);
void             qnode_mailbox_add(qnode_mailbox_t *box, struct qnode_msg_t *msg);
int              qnode_mailbox_get(qnode_mailbox_t *box, struct qnode_list_t *list);

#endif  /* __QMAILBOX_H__ */
