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
qmailbox_init(qmailbox_t *box, qmsg_pt *func,
              qengine_t *engine, void *reader) {
  qmutex_init(&(box->mutex));
  qlist_entry_init(&(box->lists[0]));
  qlist_entry_init(&(box->lists[1]));

  box->write    = &(box->lists[0]);
  box->read     = &(box->lists[1]);
  box->reader   = reader;
  box->handler  = func;
  box->done     = NULL;
  qsignal_init(&(box->signal), box, engine);
}

void
qmailbox_free(qmailbox_t *box) {
  int      i;
  qlist_t *list;
  qmsg_t  *msg;
  qlist_t *pos, *next;

  qmutex_lock(&(box->mutex));
  for (i = 0; i < 2; ++i) {
    list = &(box->lists[i]);
    for (pos = list->next; pos != list; ) {
      next = pos->next;
      qassert(next != pos);
      msg = qlist_entry(pos, qmsg_t, entry);
      pos = next;
      qmsg_destroy(msg);
    }
  }
  qmutex_unlock(&(box->mutex));
  qmutex_destroy(&(box->mutex));
  qsignal_free(&(box->signal));
}

void
qmailbox_add(qmailbox_t *box, qmsg_t *msg) {
  qmutex_lock(&(box->mutex));
  qlist_add_tail(&(msg->entry), box->write);
  qassert(msg->entry.next == box->write);
  qmutex_unlock(&(box->mutex));

  if (qsignal_active(&(box->signal), 1) == 0) {
    qsignal_send(&(box->signal));
  }
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

  if (box->done) {
    box->done(box->reader);
  }
  qassert(qlist_empty(read));
}
