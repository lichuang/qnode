/*
 * See Copyright Notice in qnode.h
 */

#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "qacceptor.h"
#include "qalloc.h"
#include "qassert.h"
#include "qlog.h"
#include "qmsg.h"
#include "qmailbox.h"

void
qmailbox_init(qmailbox_t *box, qacceptor_t *acceptor) {
  qmutex_init(&(box->mutex));
  qlist_entry_init(&(box->lists[0]));
  qlist_entry_init(&(box->lists[1]));

  box->write    = &(box->lists[0]);
  box->read     = &(box->lists[1]);
  box->acceptor = acceptor;
  qsignal_init(&(box->signal), box);
}

void
qmailbox_add(qmailbox_t *box, qmsg_t *msg) {
  qlist_t *p;

  qmutex_lock(&(box->mutex));
  p = box->write;
  qlist_add_tail(&(msg->entry), p);
  if (qsignal_active(&(box->signal), 1) == 0) {
    qsignal_send(&(box->signal));
  }
  qmutex_unlock(&(box->mutex));
}

void
qmailbox_handle(qmailbox_t *box) {
  qlist_t     *read;
  qlist_t     *pos, *next;
  qmsg_t      *msg;
  qacceptor_t *acceptor;

  acceptor = box->acceptor;
  qmutex_lock(&(box->mutex));
  read = box->write;
  box->write = box->read;
  box->read  = read;
  qmutex_unlock(&(box->mutex));

  for (pos = read->next; pos != read; ) {
    msg = qlist_entry(pos, qmsg_t, entry);
    next = pos->next;
    qlist_del_init(&(msg->entry));
    if (msg == NULL) {
      goto next;
    }
    qinfo("handle %d msg", msg->type);

    msg->handled = 1;
    if (acceptor->handle(msg, acceptor->owner) == 0) {
      qmsg_destroy(msg);
    }

next:
    pos = next;
  }
}
