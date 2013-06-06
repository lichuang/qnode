/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qassert.h"
#include "qactor.h"
#include "qcore.h"
#include "qdefines.h"
#include "qlist.h"
#include "qlog.h"
#include "qluautil.h"
#include "qmsg.h"
#include "qmmsg.h"
#include "qserver.h"
#include "qworker.h"
#include "qwmsg.h"

static int server_handle_spawn_msg(qmsg_t *msg, void *reader);
static int server_handle_signal_msg(qmsg_t *msg, void *reader);

qmsg_func_t* g_server_msg_handlers[] = {
  &server_handle_spawn_msg,   /* spawn */
  &server_handle_signal_msg,  /* signal */
};

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
  spawn   = (qmmsg_spawn_t*)new_msg;
  actor->parent = parent;
  spawn->actor = actor;
  spawn->sender = QMAIN_THREAD_TID;
  spawn->recver = qserver_worker();
  spawn->type   = W_SPAWN;
  qworker_send(spawn->recver, new_msg);
  server->actors[aid] = actor;

  return 0;
}

static int
server_handle_signal_msg(qmsg_t *msg, void *reader) {
  int             signo;
  qmmsg_signal_t *signal;
  
  UNUSED(reader);
  signal = (qmmsg_signal_t*)msg;
  signo = signal->signo;

  return 0;
}

