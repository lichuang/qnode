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

qnode_thread_t* qnode_thread_new(struct qnode_server_t *server, int tid) {
  qnode_thread_t *thread = qnode_alloc_type(qnode_thread_t);
  if (thread == NULL) {
    qnode_error("create thread error");
    return NULL;
  }
  thread->engine = qnode_engine_new();
  if (thread->engine == NULL) {
    qnode_error("create thread engine error");
    qnode_free(thread);
    return NULL;
  }
  thread->server = server;
  thread->tid = tid;
  thread->server_box = qnode_server_get_box(server, tid);
  thread->box = qnode_mailbox_new(thread_box, thread);
  qnode_assert(thread->box);
  return thread;
}

struct qnode_mailbox_t* qnode_thread_mailbox(qnode_thread_t *thread) {
  return thread->box;
}
