/*
 * See Copyright Notice in qnode.h
 */

#include "qlmsg.h"

qmsg_t*
qlmsg_log_new(qlog_t *log, int sender) {
  qmsg_t      *msg;
  qlmsg_log_t *lmsg;

  msg = qmsg_new(sender, 0, sizeof(qlmsg_log_t), L_LOG);
  if (msg == NULL) {
    return NULL;
  }

  msg->type = L_LOG;
  lmsg = (qlmsg_log_t*)msg;
  lmsg->log = log;

  return msg;
}
