/*
 * See Copyright Notice in qnode.h
 */

#include "qactor.h"
#include "qdefines.h"
#include "qmmsg.h"

qmsg_t*
qmmsg_signal_new(int signo) {
  qmsg_t         *msg;
  qmmsg_signal_t *signal;

  msg = qmsg_new(QMAINTHREAD_TID, QMAINTHREAD_TID,
                 sizeof(qmmsg_signal_t), M_SIGNAL);
  if (msg == NULL) {
    return NULL;
  }
  signal        = (qmmsg_signal_t*)msg;
  signal->signo = signo;

  return msg;
}
