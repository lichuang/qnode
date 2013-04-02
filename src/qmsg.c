/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qassert.h"
#include "qdefines.h"
#include "qlog.h"
#include "qmailbox.h"
#include "qmsg.h"
#include "qserver.h"
#include "qstring.h"
#include "qthread.h"

qmsg_t* qmsg_new(qtid_t sender_id, qtid_t receiver_id) {
  qmsg_t *msg;

  msg = malloc(sizeof(qmsg_t));
  if (msg == NULL) {
    return NULL;
  }
  qlist_entry_init(&(msg->entry));
  msg->sender_id = sender_id;
  msg->receiver_id = receiver_id;
  msg->flag = msg->type = msg->handled = 0;
  return msg;
}

void qmsg_destroy(qmsg_t *msg) {
  switch (msg->type) {
    case s_start:
      break;
    case spawn:
      break;
    case t_send:
      if (msg->handled == 0) {
        qactor_msg_destroy(msg->args.t_send.actor_msg);
      }
      break;
    default:
      break;
  }
  free(msg);
}

qactor_msg_t* qactor_msg_new() {
  return malloc(sizeof(qactor_msg_t));
}

void qactor_msg_destroy(qactor_msg_t *msg) {
  qdict_destroy(msg->arg_dict);
  free(msg);
}

qmsg_t* qmsg_clone(qmsg_t *msg) {
  qmsg_t *new_msg;

  new_msg = qmsg_new(msg->sender_id, msg->receiver_id);
  memcpy(new_msg, msg, sizeof(qmsg_t));

  return new_msg;
}

void qmsg_send(qmsg_t *msg) {
  qtid_t      sender_id, receiver_id;
  qthread_t  *thread;

  sender_id = msg->sender_id;
  receiver_id = msg->receiver_id;

  qassert(msg->type > 0 && msg->type < QMAX_MSG_TYPE);
  qinfo("add a msg %p, type: %d, sender: %d, receiver: %d, flag: %d",
        msg, msg->type, sender_id, receiver_id, msg->flag);
 
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
    thread = g_server->threads[sender_id];
    g_thread_msg_handlers[msg->type](thread, msg);
    free(msg);
    return;
  } 
  
  /* worker thread send to worker thread */
  thread = g_server->threads[sender_id];
  qmailbox_add(thread->out_box[receiver_id], msg);
}
