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
#include "qworker.h"
#include "qthread_log.h"

static int worker_msg_handler(qmsg_t *msg, void *reader);
static void* worker_main_loop(void *arg);

extern qmsg_func_t* g_worker_msg_handlers[];

static int
worker_msg_handler(qmsg_t *msg, void *reader) {
  return (*g_worker_msg_handlers[msg->type])(msg, reader);
}

static void*
worker_main_loop(void *arg) {
  qworker_t *worker;

  worker = (qworker_t*)arg;
  /* init the worker thread log structure */
  g_server->thread_log[worker->tid] = qthread_log_init(worker->tid);
  qserver_worker_started();
  qengine_loop(worker->engine);
  return NULL;
}

qworker_t*
qworker_new(qid_t tid) {
  qworker_t *worker;

  worker = qcalloc(sizeof(qworker_t));
  if (worker == NULL) {
    qerror("create worker error");
    return NULL;
  }
  worker->engine = qengine_new();
  if (worker->engine == NULL) {
    qerror("create worker engine error");
    return NULL;
  }
  qmailbox_init(&(worker->box), worker_msg_handler,
                worker->engine, worker);
  worker->tid = tid;

  /* create the lua VM for the worker */
  worker->state = qlua_new_state();
  /* init the actor list */
  qlist_entry_init(&(worker->actor_list));
  pthread_create(&worker->id, NULL,
                 worker_main_loop, worker);

  return worker;
}

void
qworker_destroy(qworker_t *worker) {
  /* wait for the thread stop */
  pthread_join(worker->id, NULL);
}

void
qworker_send(qid_t tid, qmsg_t *msg) {
  qworker_t  *worker;
  qmailbox_t *box;

  worker = g_server->workers[tid];
  box    = &(worker->box);
  qmailbox_add(box, msg);
}
