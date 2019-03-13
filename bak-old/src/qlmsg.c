/*
 * See Copyright Notice in qnode.h
 */

#include "qdefines.h"
#include "qlogger.h"
#include "qlmsg.h"

qmsg_t*
qlmsg_log_new(qlog_t *log, int sender) {
  qmsg_t      *msg;
  qlmsg_log_t *lmsg;

  msg = qmsg_new(sender, 0, sizeof(qlmsg_log_t), L_LOG);
  if (msg == NULL) {
    return NULL;
  }

  lmsg = (qlmsg_log_t*)msg;
  lmsg->log = log;

  return msg;
}

qmsg_t*
qlmsg_signal_new(int signo) {
  qmsg_t         *msg;
  qlmsg_signal_t *signal;

  msg = qmsg_new(QMAINTHREAD_TID, 0,
                 sizeof(qlmsg_signal_t), L_SIGNAL);
  if (msg == NULL) {
    return NULL;
  }

  signal        = (qlmsg_signal_t*)msg;
  signal->signo = signo;

  return msg;
}
