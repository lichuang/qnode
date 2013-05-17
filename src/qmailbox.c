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
#include "qchannel.h"

qmailbox_t* qmailbox_new(qevent_func_t *callback, void *reader) {
  qmailbox_t  *box;
  qchannel_t   *channel;

  box = qcalloc(sizeof(qmailbox_t));
  if (box == NULL) {
    return NULL;
  }
  qlist_entry_init(&(box->lists[0]));
  qlist_entry_init(&(box->lists[1]));
  box->write  = &(box->lists[0]);
  box->read   = &(box->lists[1]);
  box->callback = callback;
  box->reader = reader;
  channel = qchannel_new();
  if (channel == NULL) {
    qfree(box);
    return NULL;
  }
  box->channel = channel;
  box->active = 1;
  return box;
}

int qmailbox_active(qengine_t *engine, qmailbox_t *box) {
  int            fd;
  void          *data;
  qevent_func_t *callback;

  fd = qchannel_get_fd(box->channel);
  callback = box->callback;
  data = box->reader;

  return qengine_add_event(engine, fd, QEVENT_READ, callback, data);
}

void qmailbox_add(qmailbox_t *box, struct qmsg_t *msg) {
  qlist_t *p;

  if (!box->active) {
    qmsg_destroy(msg);
    return;
  }
  if (box->write == NULL) {
    qassert(box->write);
  }
  /* 
   * save the write ptr first cause add_tail below
   * is-not atomic operation and the write ptr maybe changed 
   */
  p = box->write;
  qlist_add_tail(&(msg->entry), p);
  if (qchannel_active(box->channel, 1) == 0) {
    qchannel_send(box->channel);
  }
  qassert(box->channel->active == 1 || box->channel->active == 0);
}

int qmailbox_get(qmailbox_t *box, qlist_t **list) {
  qassert(box->write);
  qlist_t *read;

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
  if (qchannel_active(box->channel, 0) == 1) {
    qchannel_recv(box->channel);
  }
  return qlist_empty(*list);
}
