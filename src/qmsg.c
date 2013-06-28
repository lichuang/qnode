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
qmsg_new(qid_t sender, qid_t recver, int size, int type) {
  qmsg_t *msg;

  msg = (qmsg_t*)qcalloc(size);
  if (msg == NULL) {
    qerror("alloc type %d msg error", type);
    return NULL;
  }
  qlist_entry_init(&(msg->entry));
  msg->sender   = sender;
  msg->recver   = recver;
  msg->type     = type;
  msg->size     = size;
  msg->destroy  = NULL;

  return msg;
}

void
qmsg_destroy(qmsg_t *msg) {
  if (msg->destroy) {
    (*msg->destroy)(msg);
  }
  qfree(msg);
}

qmsg_t*
qmsg_clone(qmsg_t *msg) {
  qmsg_t *new_msg;

  new_msg = qmsg_new(msg->sender, msg->recver, msg->size, msg->type);
  memcpy(new_msg, msg, msg->size);

  return new_msg;
}

void
qmsg_send(qmsg_t *msg) {
  UNUSED(msg);
}
