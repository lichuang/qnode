/*
 * See Copyright Notice in qnode.h
 */

#include <signal.h>
#include <stdio.h>
#include "qassert.h"
#include "qactor.h"
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

static int  server_handle_signal_msg(qmsg_t *msg, void *reader);

static void send_signal(int signo);

qmsg_func_t* g_server_msg_handlers[] = {
  &server_handle_signal_msg,  /* signal */
};

#if 0
static int
server_handle_spawn_msg(qmsg_t *msg, void *reader) {
  qid_t           parent;
  qserver_t      *server;
  qmsg_t         *new_msg;
  qmmsg_spawn_t  *spawn;
  qid_t           aid;
  lua_State      *state;
  qactor_t       *actor;

  qinfo("handle spawn msg");

  server  = (qserver_t*)reader;
  spawn   = (qmmsg_spawn_t*)msg;
  new_msg = qmsg_clone(msg);
  aid     = spawn->aid;
  state   = spawn->state;
  actor   = qactor_new(aid);
  parent  = spawn->parent;

  qactor_attach(actor, state);
  spawn         = (qmmsg_spawn_t*)new_msg;
  actor->parent = parent;
  spawn->actor  = actor;
  spawn->sender = QMAIN_THREAD_TID;
  spawn->recver = qserver_worker();
  spawn->type   = W_SPAWN;
  qworker_send(spawn->recver, new_msg);
  server->actors[aid] = actor;

  return 0;
}
#endif

static int
server_handle_signal_msg(qmsg_t *msg, void *reader) {
  int             signo;
  qmmsg_signal_t *signal;
  qserver_t      *server;
  
  server = (qserver_t*)reader;
  signal = (qmmsg_signal_t*)msg;
  signo  = signal->signo;

  qinfo("caugth signal %d", signo);

  switch (signo) {
    case SIGTERM:
    case SIGQUIT:
    case SIGINT:
      server->engine->quit = 1;
      send_signal(signo);
      break;
    default:
      break;
  }

  return 0;
}

static void
send_signal(int signo) {
  int         i;
  qworker_t  *worker;
  qmsg_t     *msg;

  for (i = 1; i <= g_server->config->thread_num; ++i) {
    worker = g_server->workers[i];
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
