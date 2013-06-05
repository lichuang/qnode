/*
 * See Copyright Notice in qnode.h
 */

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
#include "qworker.h"

static int
worker_handle_start_msg(qmsg_t *msg, void *reader) {
  qworker_t  *worker;
  int         ret;
  qid_t       aid;
  qactor_t   *actor;
  lua_State  *state;

  qinfo("handle start msg");

  worker = (qworker_t*)reader;
  aid = msg->args.start.aid;
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
worker_handle_spawn_msg(qmsg_t *msg, void *reader) {
  int        ret;
  qactor_t  *actor;
  qworker_t *worker;

  qinfo("handle spawn msg");

  worker = (qworker_t*)reader;
  actor = msg->args.spawn.actor;
  actor->state = msg->args.spawn.state;
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

static int worker_handle_wrong_msg(qmsg_t *msg, void *reader) {
  UNUSED(reader);
  UNUSED(msg);
  qerror("handle worker type %d msg error", msg->type);
  return 0;
}

qmsg_func_t* g_worker_msg_handlers[] = {
  &worker_handle_wrong_msg,     /* WRONG */
  &worker_handle_start_msg,     /* start */
  &worker_handle_spawn_msg,     /* spawn */
  &worker_handle_send_msg,      /* send */
};
