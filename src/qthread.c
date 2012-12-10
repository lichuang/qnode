/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qassert.h"
#include "qengine.h"
#include "qlog.h"
#include "qmailbox.h"
#include "qmalloc.h"
#include "qserver.h"
#include "qthread.h"

static void thread_box(int fd, int flags, void *data) {
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
  int result;
  result = pthread_create(&thread->id, NULL, main_loop, thread);
  qcheck(result == 0);
  return thread;
}

struct qmailbox_t* qthread_mailbox(qthread_t *thread) {
  return thread->box;
}
