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

struct qmailbox_t {
  qlist_t           lists[2];   /* one for read, one for write */
  qlist_t          *write;      /* current write list ptr */
  qlist_t          *read;       /* current read  list ptr */
  void             *reader;     /* mailbox reader */
  qmutex_t          mutex;      /* mutex for protect msg */
  qsignal_t         signal;     /* signaler */
  qacceptor_t      *acceptor;   /* owner acceptor */
};

void  qmailbox_add(qmailbox_t *box, qmsg_t *msg);
void  qmailbox_init(qmailbox_t *box, qacceptor_t *acceptor);
void  qmailbox_handle(qmailbox_t *box);

#endif  /* __QMAILBOX_H__ */
