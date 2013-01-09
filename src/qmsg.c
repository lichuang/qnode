/*
 * See Copyright Notice in qnode.h
 */

#include <string.h>
#include <stdio.h>
#include "qassert.h"
#include "qdefines.h"
#include "qlog.h"
#include "qmailbox.h"
#include "qmalloc.h"
#include "qmsg.h"
#include "qserver.h"
#include "qthread.h"

qmsg_t* qmsg_new(qtid_t sender_id, qtid_t receiver_id) {
  qmsg_t *msg = qalloc_type(qmsg_t);
  if (msg == NULL) {
    return NULL;
  }
  qlist_entry_init(&(msg->entry));
  msg->sender_id = sender_id;
  msg->receiver_id = receiver_id;
  msg->flag = msg->type = 0;
  return msg;
}

qactor_msg_t* qactor_msg_new() {
  qactor_msg_t *msg = qalloc_type(qactor_msg_t);
  qlist_entry_init(&msg->head);
  return msg;
}

void qactor_msg_destroy(qactor_msg_t *msg) {
  UNUSED(msg);
}

qarg_t* qarg_new() {
  qarg_t *arg = qalloc_type(qarg_t);
  qlist_entry_init(&arg->entry);
  return arg;
}

qmsg_t* qmsg_clone(qmsg_t *msg) {
  qmsg_t *new_msg = qmsg_new(msg->sender_id, msg->receiver_id);
  memcpy(new_msg, msg, sizeof(qmsg_t));
  return new_msg;
}

void qmsg_send(qmsg_t *msg) {
  qassert(msg);
  qtid_t sender_id = msg->sender_id;
  qtid_t receiver_id = msg->receiver_id;
  qassert(msg->type > 0 && msg->type < QMAX_MSG_TYPE);
  qinfo("add a msg %p, type: %d, sender: %d, receiver: %d, flag: %d", msg, msg->type, sender_id, receiver_id, msg->flag);
 
  /* server thread send to worker thread */
  if (sender_id == QSERVER_THREAD_TID) {
    qassert(receiver_id != QSERVER_THREAD_TID);
    qmailbox_add(g_server->out_box[receiver_id], msg);
    return;
  }

  /* worker thread send to server thread */
  if (receiver_id == QSERVER_THREAD_TID) {
    qassert(sender_id != QSERVER_THREAD_TID);
    qmailbox_add(g_server->in_box[receiver_id], msg);
    return;
  }

  /* sender and receiver is the same worker thread */
  if (sender_id == receiver_id) {
    qassert(sender_id != QSERVER_THREAD_TID);
    qthread_t *thread = g_server->threads[sender_id];
    g_thread_msg_handlers[msg->type](thread, msg);
    qfree(msg);
    return;
  } 
  
  /* worker thread send to worker thread */
  qthread_t *thread = g_server->threads[sender_id];
  qmailbox_add(thread->out_box[receiver_id], msg);
}
