/*
 * See Copyright Notice in qnode.h
 */

#include <signal.h>
#include <stdio.h>
#include "qassert.h"
#include "qactor.h"
#include "qamsg.h"
#include "qconfig.h"
#include "qdefines.h"
#include "qlist.h"
#include "qlog.h"
#include "qluautil.h"
#include "qmailbox.h"
#include "qmsg.h"
#include "qserver.h"
#include "qwmsg.h"
#include "qworker.h"

static int worker_start_handler(qmsg_t *msg, void *reader);
static int worker_spawn_handler(qmsg_t *msg, void *reader);
static int worker_signal_handler(qmsg_t *msg, void *reader);
static int worker_actor_handler(qmsg_t *msg, void *reader);

qmsg_pt* worker_msg_handlers[] = {
  &worker_start_handler,
  &worker_spawn_handler,
  &worker_signal_handler,
  &worker_actor_handler,
};

static int
worker_start_handler(qmsg_t *msg, void *reader) {
  qwmsg_start_t  *start;
  qworker_t      *worker;
  int             ret;
  qid_t           aid;
  qactor_t       *actor;
  lua_State      *state;

  qinfo("handle start msg");

  worker = (qworker_t*)reader;
  start  = (qwmsg_start_t*)msg;
  aid    = start->aid;
  actor  = qactor_new(aid);
  if (actor == NULL) {
    qerror("new actor: %d error", aid);
    return QERROR;
  }
  qassert(actor->state == NULL);
  qactor_attach(actor, qlua_new_thread(worker));
  actor->tid = worker->tid;

  if (qlua_threadloadfile(actor, actor->state, config.main) != 0) {
    qerror("load server start script %s error", config.main);
    return QERROR;
  }

  state = actor->state;
  lua_getglobal(state, "server");
  if (lua_isnil(state, -1)) {
    qerror("load server script server table error");
    return QERROR;
  }
  lua_getfield(state, -1, "start");
  if (lua_isnil(state, -1)) {
    qerror("load server script server.start func error");
    return QERROR;
  }
  if (qlua_call(state, 0, 0) == 0) {
    ret = (int)lua_tonumber(state, -1);
    lua_pop(state, 1 );
  } else {
    qlua_fail(state);
  }

  return ret;
}

static int
worker_spawn_handler(qmsg_t *msg, void *reader) {
  int             ret;
  qactor_t       *actor;
  qworker_t      *worker;
  qwmsg_spawn_t  *spawn;

  qinfo("handle spawn msg");

  spawn  = (qwmsg_spawn_t*)msg;
  worker = (qworker_t*)reader;
  actor = spawn->actor;
  actor->state = spawn->state;
  actor->tid = worker->tid;
  lua_State *state = actor->state;
  if (qlua_call(state, 1, 0) == 0) {
    ret = (int)lua_tonumber(state, -1);
    lua_pop(state, 1 );
  } else {
    qlua_fail(state);
  }

  return ret;
}

static int
worker_signal_handler(qmsg_t *msg, void *reader) {
  qwmsg_signal_t *signal;
  qworker_t      *worker;

  signal = (qwmsg_signal_t*)msg;
  worker = (qworker_t*)reader;
  
  switch (signal->signo) {
    case SIGTERM:
    case SIGQUIT:
    case SIGINT:
      worker->engine->quit = 1;
      break;
    case SIGUSR1:
      qlua_reload(worker->state, NULL);
      break;
    default:
      break;
  }

  return QOK;
}

static int
worker_actor_handler(qmsg_t *msg, void *reader) {
  qwmsg_actor_t   *amsg;
  qworker_t       *worker;
  qamsg_header_t  *header;
  qactor_t        *actor;

  amsg   = (qwmsg_actor_t*)msg;
  worker = (qworker_t*)reader;
  header = amsg->data;
  actor  = qworket_get_actor(worker, decode_id(header->dst));
  if (actor == NULL) {
    qerror("actor %d not exist", header->dst);
    return QOK;
  }

  (*actor_msg_handlers[header->type])(header, actor);

  return 0;
}
