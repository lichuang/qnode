/*
 * See Copyright Notice in qnode.h
 */

#include <unistd.h>
#include <stdio.h>
#include "qalloc.h"
#include "qassert.h"
#include "qactor.h"
#include "qconfig.h"
#include "qengine.h"
#include "qdefines.h"
#include "qlist.h"
#include "qlog.h"
#include "qluautil.h"
#include "qserver.h"
#include "qthread.h"
#include "qthread_log.h"

static int worker_msg_handler(qmsg_t *msg, void *reader);
static void* worker_thread_main_loop(void *arg);

extern qthread_msg_handler g_thread_msg_handlers[];

static int
worker_msg_handler(qmsg_t *msg, void *reader) {
  qthread_t *thread;

  thread = (qthread_t*)reader;
  qinfo("handle %d msg", msg->type);
  return g_thread_msg_handlers[msg->type](thread, msg);
}

static void*
worker_thread_main_loop(void *arg) {
  qthread_t *thread;

  thread = (qthread_t*)arg;
  /* init the worker thread log structure */
  g_server->thread_log[thread->tid] = qthread_log_init(thread->tid);
  qserver_worker_started();
  qengine_loop(thread->engine);
  return NULL;
}

qthread_t*
qthread_new(qtid_t tid) {
  qthread_t    *thread;

  thread = qcalloc(sizeof(qthread_t));
  if (thread == NULL) {
    qerror("create thread error");
    return NULL;
  }
  thread->engine = qengine_new();
  if (thread->engine == NULL) {
    qerror("create thread engine error");
    return NULL;
  }
  qmailbox_init(&(thread->box), worker_msg_handler,
                thread->engine, thread);
  thread->tid = tid;

  /* create the lua VM for the thread */
  thread->state = qlua_new_state();
  /* init the actor list */
  qlist_entry_init(&(thread->actor_list));
  pthread_create(&thread->id, NULL,
                 worker_thread_main_loop, thread);

  return thread;
}

void
qthread_destroy(qthread_t *thread) {
  /* wait for the thread stop */
  pthread_join(thread->id, NULL);
}

void
qthread_send(qtid_t tid, qmsg_t *msg) {
  qthread_t  *thread;
  qmailbox_t *box;

  thread = g_server->threads[tid];
  box    = &(thread->box);
  qmailbox_add(box, msg);
}
