/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QMAILBOX_H__
#define __QMAILBOX_H__

#include "qatomic.h"
#include "qcore.h"
#include "qengine.h"
#include "qlist.h"
#include "qmutex.h"
#include "qsignal.h"

typedef void (qmsg_done_t)(void *reader);

struct qmailbox_t {
  qlist_t           lists[2];   /* one for read, one for write */
  qlist_t          *write;      /* current write list ptr */
  qlist_t          *read;       /* current read  list ptr */
  void             *reader;     /* mailbox reader */
  qmutex_t          mutex;      /* mutex for protect msg */
  qsignal_t         signal;     /* signaler */
  qmsg_func_t      *handler;    /* per-msg handler */
  qmsg_done_t      *done;       /* when handle msg list done callback */
};

void  qmailbox_init(qmailbox_t *box, qmsg_func_t *func,
                    qengine_t *engine, void *reader);
void  qmailbox_add(qmailbox_t *box, qmsg_t *msg);
void  qmailbox_handle(qmailbox_t *box);

#endif  /* __QMAILBOX_H__ */
