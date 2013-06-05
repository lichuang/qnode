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
#include "qalloc.h"
#include "qassert.h"
#include "qlog.h"
#include "qmsg.h"
#include "qmailbox.h"

void
qmailbox_init(qmailbox_t *box, qmsg_func_t *func,
              qengine_t *engine, void *reader) {
  qmutex_init(&(box->mutex));
  qlist_entry_init(&(box->lists[0]));
  qlist_entry_init(&(box->lists[1]));

  box->write    = &(box->lists[0]);
  box->read     = &(box->lists[1]);
  box->reader   = reader;
  box->handler  = func;
  qsignal_init(&(box->signal), box, engine);
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

  qmutex_lock(&(box->mutex));
  read       = box->write;
  box->write = box->read;
  box->read  = read;
  qmutex_unlock(&(box->mutex));

  for (pos = read->next; pos != read; ) {
    msg = qlist_entry(pos, qmsg_t, entry);
    next = pos->next;
    qlist_del_init(&(msg->entry));

    if (box->handler(msg, box->reader) == 0) {
      qmsg_destroy(msg);
    }

    pos = next;
  }
}
