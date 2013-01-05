/*
 * See Copyright Notice in qnode.h
 */

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
  msg->flag = msg->type = msg->mask = 0;
  return msg;
}
