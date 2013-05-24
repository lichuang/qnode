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
#include "qmailbox.h"
#include "qserver.h"
#include "qthread.h"
#include "qthread_log.h"

static void thread_box(int fd, int flags, void *data) {
  UNUSED(fd);
  UNUSED(flags);

  qmsg_t      *msg;
  qlist_t     *list;
  qlist_t     *pos, *next;
  qthread_t   *thread;

  thread = (qthread_t*)data;
  qmailbox_get(thread->box, &list);
  for (pos = list->next; pos != list; ) {
    msg = qlist_entry(pos, qmsg_t, entry);
    qassert(msg);
    next = pos->next;
    qlist_del_init(&(msg->entry));
    if (msg == NULL) {
      qinfo("msg NULL");
      goto next;
    }
    qinfo("handle %d msg %p", msg->type, msg);
    if (!qmsg_is_smsg(msg)) {
      qerror("msg %d , flag %d is not server msg", msg->type, msg->flag);
      goto next;
    }
    if (qmsg_invalid_type(msg->type)) {
      qerror("msg %d is not valid msg type", msg->type);
      goto next;
    }
    (g_thread_msg_handlers[msg->type])(thread, msg);

next:
    msg->handled = 1;
    qmsg_destroy(msg);
    pos = next;
  }
}

static void* worker_thread_main_loop(void *arg) {
  qthread_t *thread;

  thread = (qthread_t*)arg;
  /* init the worker thread log structure */
  g_server->thread_log[thread->tid] = qthread_log_init(thread->tid);
  qserver_worker_started();
  qengine_loop(thread->engine);
  return NULL;
}

qthread_t* qthread_new(qtid_t tid) {
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
  thread->tid = tid;

  thread->box = qmailbox_new(thread_box, thread);
  if (thread->box == NULL) {
    qerror("create thread box error");
    return NULL;
  }
  qmailbox_active(thread->engine, thread->box);

  /* create the lua VM for the thread */
  thread->state = qlua_new_state();
  /* init the actor list */
  qlist_entry_init(&(thread->actor_list));
  pthread_create(&thread->id, NULL,
                 worker_thread_main_loop, thread);

  return thread;
}

void qthread_destroy(qthread_t *thread) {
  /* wait for the thread stop */
  pthread_join(thread->id, NULL);
}
