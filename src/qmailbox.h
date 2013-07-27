/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QMAILBOX_H__
#define __QMAILBOX_H__

#include "qatomic.h"
#include "qcore.h"
#include "qengine.h"
#include "qlist.h"
#include "qmsg.h"
#include "qmutex.h"
#include "qsignal.h"

typedef void (qmsg_done_pt)(void *reader);

struct qmailbox_t {
  const char       *name;

  /* one for read, one for write */
  qlist_t           lists[2];

  /* current write list ptr */
  qlist_t          *write;

  /* current read  list ptr */
  qlist_t          *read;

  /* mailbox reader */
  void             *reader;

  /* mutex for protect msg */
  qmutex_t          mutex;

  /* signaler */
  qsignal_t         signal;

  /* per-msg handler */
  qmsg_pt          *handler;

  /* when handle msg list done callback */
  qmsg_done_pt     *done;
};

void  qmailbox_init(qmailbox_t *box, qmsg_pt *func,
                    qengine_t *engine, void *reader);
void  qmailbox_free(qmailbox_t *box);
void  qmailbox_add(qmailbox_t *box, qmsg_t *msg);
void  qmailbox_handle(qmailbox_t *box);

#endif  /* __QMAILBOX_H__ */
