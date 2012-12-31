/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QMAILBOX_H__
#define __QMAILBOX_H__

#include "qengine.h"
#include "qlist.h"

struct qmsg_t;
struct qsignal_t;

typedef struct qmailbox_t {
  struct qlist_t lists[2]; /* one for read, one for write */
  struct qlist_t *write;   /* current write list */
  struct qlist_t *read;    /* current write list */
  qevent_func_t *callback; /* mailbox reader callback */
  void *reader;            /* mailbox reader */
  struct qsignal_t *signal;
} qmailbox_t;

qmailbox_t* qmailbox_new(qevent_func_t *callback, void *reader);
void        qmailbox_destroy(qmailbox_t *box);
int         qmailbox_active(qengine_t *engine, qmailbox_t *box);
void        qmailbox_add(qmailbox_t *box, struct qmsg_t *msg);
int         qmailbox_get(qmailbox_t *box, qlist_t **list);

#endif  /* __QMAILBOX_H__ */
