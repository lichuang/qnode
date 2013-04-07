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
#include "qthread.h"

static int thread_handle_sstart_msg(qthread_t *thread, qmsg_t *msg) {
  qinfo("handle start msg");

  int         ret;
  qid_t       aid;
  qactor_t   *actor;
  lua_State  *state;

  aid = msg->args.s_start.aid;
  actor = qactor_new(aid);
  if (actor == NULL) {
    qerror("new actor: %d error", aid);
    return -1;
  }
  qassert(actor->state == NULL);
  qactor_attach(actor, qlua_new_thread(thread));
  actor->tid = thread->tid;

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

static int thread_handle_sstop_msg(qthread_t *thread, qmsg_t *msg) {
  UNUSED(msg);
  thread->running = 0;
  return 0;
}

static int thread_handle_spawn_msg(qthread_t *thread, qmsg_t *msg) {
  qinfo("handle spawn msg");

  int       ret;
  qactor_t *actor;

  actor = msg->args.spawn.actor;
  actor->state = msg->args.spawn.state;
  actor->tid = thread->tid;
  lua_State *state = actor->state;
  if (qlua_call(state, 1, 0) == 0) {
    ret = (int)lua_tonumber(state, -1);
    lua_pop(state, 1 );
  } else {
    qlua_fail(state, __FILE__, __LINE__);
  }

  return ret;
}

static int thread_handle_tsend_msg(qthread_t *thread, qmsg_t *msg) {
  UNUSED(thread);
  qinfo("handle tsend msg");

  qactor_t      *actor;
  qactor_msg_t  *actor_msg;
  lua_State     *state;

  actor_msg = msg->args.t_send.actor_msg;
  actor = qserver_get_actor(actor_msg->dst);
  state = actor->state;

  /*
   * if the state yield waiting for msg, push the msg into stack and resume
   */
  if (lua_status(state) == LUA_YIELD && actor->waiting_msg) {
    actor->waiting_msg = 0;
    lua_newtable(state);
    qlua_dump_dict(state, actor_msg->arg_dict);
    return lua_resume(state, 1);
  }
  /*
   * else add the msg to the actor msg list
   */
  qlist_add_tail(&actor_msg->entry, &(actor->msg_list));

  return 0;
}

static int thread_handle_wrong_msg(qthread_t *thread, qmsg_t *msg) {
  UNUSED(thread);
  UNUSED(msg);
  qerror("handle thread type %d msg error", msg->type);
  return 0;
}

qthread_msg_handler g_thread_msg_handlers[] = {
  &thread_handle_wrong_msg,     /* WRONG */
  &thread_handle_sstart_msg,    /* s_start */
  &thread_handle_sstop_msg,     /* s_stop */
  &thread_handle_spawn_msg,     /* spawn */
  &thread_handle_tsend_msg,     /* t_send */
};
