/*
 * See Copyright Notice in qnode.h
 */

#include <signal.h>
#include <stdio.h>
#include "qassert.h"
#include "qactor.h"
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

qmsg_func_t* g_worker_msg_handlers[] = {
  &worker_start_handler,
  &worker_spawn_handler,
  &worker_signal_handler,
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
  aid = start->aid;
  actor = qactor_new(aid);
  if (actor == NULL) {
    qerror("new actor: %d error", aid);
    return -1;
  }
  qassert(actor->state == NULL);
  qactor_attach(actor, qlua_new_thread(worker));
  actor->tid = worker->tid;

  if (qlua_threadloadfile(actor, actor->state, "server.lua") != 0) {
    qerror("load server start script error");
    return -1; 
  }
  state = actor->state;
  lua_getglobal(state, "server");
  if (lua_isnil(state, -1)) {
    qerror("load server start script error");
    return -1;
  }
  lua_getfield(state, -1, "start");
  if (lua_isnil(state, -1)) {
    qerror("load server start script error");
    return -1;
  }
  if (qlua_call(state, 0, 0) == 0) {
    ret = (int)lua_tonumber(state, -1);
    lua_pop(state, 1 );
  } else {
    qlua_fail(state, __FILE__, __LINE__);
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
    qlua_fail(state, __FILE__, __LINE__);
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
      worker->engine->quit = 1;
      break;
    default:
      break;
  }

  return 0;
}

#if 0
static int
worker_handle_send_msg(qmsg_t *msg, void *reader) {
  qactor_t      *actor;
  qactor_msg_t  *actor_msg;
  lua_State     *state;

  UNUSED(reader);
  qinfo("handle send msg");

  actor_msg = msg->args.send.actor_msg;
  actor = qserver_get_actor(actor_msg->dst);
  state = actor->state;

  /*
   * if the state yield waiting for msg, push the msg
   * into stack and resume
   */
  if (lua_status(state) == LUA_YIELD && actor->waiting_msg) {
    actor->waiting_msg = 0;
    lua_newtable(state);
    qlua_dump_dict(state, actor_msg->arg_dict);
    return lua_resume(state, 1);
  }
  /* else add the msg to the actor msg list */
  qlist_add_tail(&actor_msg->entry, &(actor->msg_list));

  return 0;
}
#endif
