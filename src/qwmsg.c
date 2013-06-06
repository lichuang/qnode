/*
 * See Copyright Notice in qnode.h
 */

#include "qwmsg.h"

qmsg_t* qwmsg_start_new(qid_t aid, qid_t sender, qid_t recver) {
  qmsg_t        *msg;
  qwmsg_start_t *start;

  msg = qmsg_new(sender, recver, sizeof(qwmsg_start_t), W_START);
  if (msg == NULL) {
    return NULL;
  }

  start       = (qwmsg_start_t*)msg;
  start->aid  = aid;

  return msg;
}
