/*
 * See Copyright Notice in qnode.h
 */

#include <string.h>
#include <stdio.h>
#include "qassert.h"
#include "qmalloc.h"
#include "qmsg.h"

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

qmsg_t* qmsg_clone(qmsg_t *msg) {
  qmsg_t *new_msg = qmsg_new(msg->sender_id, msg->receiver_id);
  memcpy(new_msg, msg, sizeof(qmsg_t));
  return new_msg;
}
