/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qassert.h"
#include "qactor.h"
#include "qengine.h"
#include "qlist.h"
#include "qlog.h"
#include "qluautil.h"
#include "qmailbox.h"
#include "qmalloc.h"
#include "qmsg.h"
#include "qserver.h"
#include "qthread.h"

typedef int (*msg_handler)(qthread_t *thread, qmsg_t *msg);

static int thread_handle_start_msg(qthread_t *thread, qmsg_t *msg) {
  qinfo("handle start msg");
  qactor_t *actor = msg->args.start.actor;

  qlua_init_path(actor);
  if (luaL_dofile(actor->state, "server.lua") != 0 ) {
    qerror("do file error");
  }
  /*
  if (qlua_dofile(actor, "server.lua") != 0) {
    qerror("load server start script error");
    return -1; 
  }
  */
  lua_State *state = actor->state;
  lua_getglobal(state, "server");
  lua_getfield(state, -1, "start");
  lua_call(state, 0, 0);
  return 0;
}

msg_handler handlers[] = {
  &thread_handle_start_msg,
};

static void thread_box(int fd, int flags, void *data) {
  qlist_t *list;
  qthread_t *thread = (qthread_t*)data;
  qmailbox_get(thread->box, &list);
  qlist_t *pos;
  qlist_for_each(pos, list) {
    qmsg_t *msg = qlist_entry(pos, qmsg_t, entry);
    (handlers[msg->type])(thread, msg);
  }
}

static void* main_loop(void *arg) {
  qthread_t *thread = (qthread_t*)arg;
  qengine_loop(thread->engine);
  return NULL;
}

qthread_t* qthread_new(struct qserver_t *server, int tid) {
  qthread_t *thread = qalloc_type(qthread_t);
  if (thread == NULL) {
    qerror("create thread error");
    return NULL;
  }
  thread->engine = qengine_new();
  if (thread->engine == NULL) {
    qerror("create thread engine error");
    qfree(thread);
    return NULL;
  }
  thread->server = server;
  thread->tid = tid;
  thread->server_box = qserver_get_box(server, tid);
  thread->box = qmailbox_new(thread_box, thread);
  qassert(thread->box);
  qlist_entry_init(&(thread->actor_list));
  int result;
  result = pthread_create(&thread->id, NULL, main_loop, thread);
  qcheck(result == 0);
  qmailbox_active(thread->engine, thread->box);
  return thread;
}

struct qmailbox_t* qthread_mailbox(qthread_t *thread) {
  return thread->box;
}
