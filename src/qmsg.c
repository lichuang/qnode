/*
 * See Copyright Notice in qnode.h
 */

#include "qalloc.h"
#include "qassert.h"
#include "qdefines.h"
#include "qlog.h"
#include "qmailbox.h"
#include "qmsg.h"
#include "qserver.h"
#include "qstring.h"
#include "qworker.h"

qmsg_t*
qmsg_new(qid_t sender_id, qid_t recv_id) {
  qmsg_t *msg;

  msg = qcalloc(sizeof(qmsg_t));
  if (msg == NULL) {
    return NULL;
  }
  qlist_entry_init(&(msg->entry));
  msg->sender_id = sender_id;
  msg->receiver_id = receiver_id;
  msg->type = QINVALID_MSG;

  return msg;
}

void
qmsg_destroy(qmsg_t *msg) {
  switch (msg->type) {
    case START:
      break;
    case SPAWN:
      break;
    case SEND:
      /*
      if (msg->handled == 0) {
        qactor_msg_destroy(msg->args.send.actor_msg);
      }
      */
      break;
    default:
      break;
  }
  qfree(msg);
}

qactor_msg_t*
qactor_msg_new() {
  return qalloc(sizeof(qactor_msg_t));
}

void
qactor_msg_destroy(qactor_msg_t *msg) {
  qdict_destroy(msg->arg_dict);
  qfree(msg);
}

qmsg_t*
qmsg_clone(qmsg_t *msg) {
  qmsg_t *new_msg;

  new_msg = qmsg_new(msg->sender_id, msg->receiver_id);
  memcpy(new_msg, msg, sizeof(qmsg_t));

  return new_msg;
}
