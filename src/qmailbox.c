/*
 * See Copyright Notice in qnode.h
 */

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "qassert.h"
#include "qlog.h"
#include "qmsg.h"
#include "qmailbox.h"
#include "qmalloc.h"
#include "qsignal.h"

qmailbox_t* qmailbox_new(qevent_func_t *callback, void *reader) {
  qmailbox_t *box = qalloc_type(qmailbox_t);
  qalloc_assert(box);
  qlist_entry_init(&(box->lists[0]));
  qlist_entry_init(&(box->lists[1]));
  box->write  = &(box->lists[0]);
  box->read   = &(box->lists[1]);
  box->callback = callback;
  box->reader = reader;
  qsignal_t *signal = qsignal_new();
  qalloc_assert(signal);
  box->signal = signal;
  box->active = 1;
  return box;
}

void qmailbox_destroy(qmailbox_t *box) {
  qassert(box->active == 0);
  qsignal_destroy(box->signal);
  qfree(box);
}

int qmailbox_active(qengine_t *engine, qmailbox_t *box) {
  int fd = qsignal_get_fd(box->signal);
  qevent_func_t *callback = box->callback;
  void *data = box->reader;
  return qengine_add_event(engine, fd, QEVENT_READ, callback, data);
}

void qmailbox_add(qmailbox_t *box, struct qmsg_t *msg) {
  if (!box->active) {
    qmsg_destroy(msg);
    return;
  }
  if (box->write == NULL) {
    qassert(box->write);
  }
  qinfo("qmailbox_add %p, write: %p", box, box->write);
  /* 
   * save the write ptr first cause add_tail below
   * is-not atomic operation and the write ptr maybe changed 
   * */
  qlist_t *p = box->write;
  qlist_add_tail(&(msg->entry), p);
  if (qsignal_active(box->signal, 1) == 0) {
    qsignal_send(box->signal);
  }
  qassert(box->signal->active == 1 || box->signal->active == 0);
}

int qmailbox_get(qmailbox_t *box, qlist_t **list) {
  qassert(box->write);
  qinfo("qmailbox_get %p, active: %d, write: %p", box, box->signal->active, box->write);
  *list = NULL;
  /* first save the read ptr */
  qlist_t *read = box->read;
  /* second change the read ptr to the write ptr */
  qatomic_ptr_xchg(&(box->read), box->write);
  /* last change the write ptr to the read ptr saved before and return to list */
  *list = qatomic_ptr_xchg(&(box->write), read);
  qassert(box->read != box->write);
  qassert(*list == box->read);
  if (qsignal_active(box->signal, 0) == 1) {
    qsignal_recv(box->signal);
  }
  qinfo("qmailbox_get %p, active: %d, write: %p", box, box->signal->active, *list);
  return qlist_empty(*list);
}
