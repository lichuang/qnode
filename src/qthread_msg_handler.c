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
#include "qmalloc.h"
#include "qmsg.h"
#include "qserver.h"
#include "qthread.h"

static int thread_handle_sstart_msg(qthread_t *thread, qmsg_t *msg) {
  qinfo("handle start msg");
  qactor_t *actor = msg->args.s_start.actor;
  actor->tid = thread->tid;

  if (qlua_dofile(actor->state, "server.lua") != 0) {
    qerror("load server start script error");
    return -1; 
  }
  lua_State *state = actor->state;
  lua_getglobal(state, "server");
  lua_getfield(state, -1, "start");
  lua_call(state, 0, 0);
  return 0;
}

static int thread_handle_spawn_msg(qthread_t *thread, qmsg_t *msg) {
  UNUSED(thread);
  qinfo("handle spawn msg");
  qactor_t *actor = msg->args.spawn.actor;
  actor->state = msg->args.spawn.state;
  lua_call(actor->state, 1, 0);
  return 0;
}

static int thread_handle_tsend_msg(qthread_t *thread, qmsg_t *msg) {
  UNUSED(thread);
  qinfo("handle tsend msg");
  qactor_t *actor = msg->args.spawn.actor;
  actor->state = msg->args.spawn.state;
  lua_call(actor->state, 1, 0);
  return 0;
}

static int thread_handle_wrong_msg(qthread_t *thread, qmsg_t *msg) {
  UNUSED(thread);
  UNUSED(msg);
  qerror("handle thread type %d msg error", msg->type);
  return 0;
}

qthread_msg_handler g_thread_msg_handlers[] = {
  &thread_handle_wrong_msg,     /* wrong */
  &thread_handle_sstart_msg,    /* s_start */
  &thread_handle_spawn_msg,     /* spawn */
  &thread_handle_tsend_msg,     /* t_send */
};
