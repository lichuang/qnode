/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qassert.h"
#include "qactor.h"
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
  actor->thread = thread;

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
  qinfo("handle spawn msg");
  qmsg_clear_undelete(msg);
  qactor_t *actor = msg->args.spawn.actor;
  actor->state = msg->args.spawn.state;
  lua_call(actor->state, 1, 0);
  return 0;
}

static int thread_handle_init_msg(qthread_t *thread, qmsg_t *msg) {
  qinfo("handle init msg\n");
  int thread_num = msg->args.s_init.thread_num;
  thread->thread_box = (qmailbox_t**)qmalloc(thread_num * sizeof(qmailbox_t*));
  qtid_t tid = thread->tid;
  int i = 0;
  for (i = 1; i <= thread_num; ++i) {
    if (i != tid) {
      qmailbox_t *box = qalloc_type(qmailbox_t);
      thread->thread_box[i] = box;
      qmsg_t *msg = qmsg_new();
      qmsg_init_box(msg, box, tid, i);
      qserver_add_mail(tid, msg);
    } else {
      thread->thread_box[i] = NULL;
    }
  }
  return 0;
}

static int thread_handle_wrong_msg(qthread_t *thread, qmsg_t *msg) {
  qerror("handle thread type %d msg error", msg->type);
  return 0;
}

smsg_handler smsg_handlers[] = {
  &thread_handle_wrong_msg,     /* wrong */
  &thread_handle_init_msg,      /* s_init */
  &thread_handle_wrong_msg,     /* w_init, wrong */
  &thread_handle_sstart_msg,    /* m_start */
  &thread_handle_spawn_msg,     /* spawn */
};
