/*
 * See Copyright Notice in qnode.h
 */

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "qalloc.h"
#include "qassert.h"
#include "qlog.h"
#include "qmsg.h"
#include "qmailbox.h"
#include "qsignal.h"

qmailbox_t*
qmailbox_new(qevent_func_t *callback, void *reader) {
  qmailbox_t  *box;
  qsignal_t   *signal;

  box = qcalloc(sizeof(qmailbox_t));
  if (box == NULL) {
    return NULL;
  }
  if (qmutex_init(&(box->mutex)) != 0) {
    qfree(box);
    return NULL;
  }
  qlist_entry_init(&(box->lists[0]));
  qlist_entry_init(&(box->lists[1]));
  box->write  = &(box->lists[0]);
  box->read   = &(box->lists[1]);
  box->callback = callback;
  box->reader = reader;
  signal = qsignal_new();
  if (signal == NULL) {
    qfree(box);
    return NULL;
  }
  box->signal = signal;
  box->active = 1;
  return box;
}

int
qmailbox_active(qengine_t *engine, qmailbox_t *box) {
  int            fd;
  void          *data;
  qevent_func_t *callback;

  fd = qsignal_get_fd(box->signal);
  callback = box->callback;
  data = box->reader;

  return qengine_add_event(engine, fd, QEVENT_READ, callback, data);
}

void
qmailbox_add(qmailbox_t *box, struct qmsg_t *msg) {
  qlist_t *p;

  if (!box->active) {
    qmsg_destroy(msg);
    return;
  }
  if (box->write == NULL) {
    qassert(box->write);
  }
  qmutex_lock(&(box->mutex));
  p = box->write;
  qlist_add_tail(&(msg->entry), p);
  if (qsignal_active(box->signal, 1) == 0) {
    qsignal_send(box->signal);
  }
  qassert(box->signal->active == 1 || box->signal->active == 0);
  qmutex_unlock(&(box->mutex));
}

int
qmailbox_get(qmailbox_t *box, qlist_t **list) {
  qassert(box->write);
  qlist_t *read;

  qmutex_lock(&(box->mutex));
  *list = NULL;
  /* first save the read ptr */
  read = box->read;
  /* second change the read ptr to the write ptr */
  qatomic_ptr_xchg(&(box->read), box->write);
  /* 
   * last change the write ptr to the read ptr saved before
   * and return to list
   */
  *list = qatomic_ptr_xchg(&(box->write), read);
  qassert(box->read != box->write);
  qassert(*list == box->read);
  qmutex_unlock(&(box->mutex));
  if (qsignal_active(box->signal, 0) == 1) {
    qsignal_recv(box->signal);
  }
  return qlist_empty(*list);
}
