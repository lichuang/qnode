/*
 * See Copyright Notice in qnode.h
 */

#include <signal.h>
#include <stdio.h>
#include "qassert.h"
#include "qactor.h"
#include "qconfig.h"
#include "qcore.h"
#include "qdefines.h"
#include "qlist.h"
#include "qlog.h"
#include "qluautil.h"
#include "qlmsg.h"
#include "qlogger.h"
#include "qmsg.h"
#include "qmmsg.h"
#include "qserver.h"
#include "qworker.h"
#include "qwmsg.h"

static int  server_signal_handler(qmsg_t *msg, void *reader);

static void send_signal(int signo);

qmsg_pt* server_msg_handlers[] = {
  &server_signal_handler,  /* signal */
};

static int
server_signal_handler(qmsg_t *msg, void *reader) {
  int             signo;
  qmmsg_signal_t *signal;
  
  signal = (qmmsg_signal_t*)msg;
  signo  = signal->signo;

  qinfo("caugth signal %d", signo);

  switch (signo) {
    case SIGTERM:
    case SIGQUIT:
    case SIGINT:
      qserver_quit();
      break;
    case SIGUSR1:
      break;
    default:
      break;
  }

  send_signal(signo);
  return QOK;
}

static void
send_signal(int signo) {
  int         i;
  qworker_t  *worker;
  qmsg_t     *msg;

  for (i = 1; i <= config.worker; ++i) {
    worker = workers[i];
    msg = qwmsg_signal_new(worker->tid, signo);
    if (msg == NULL) {
      continue;
    }
    qworker_send(msg);
  }
  msg = qlmsg_signal_new(signo);
  if (msg == NULL) {
    return;
  }
  qlogger_send(msg);
}
