/*
 * See Copyright Notice in qnode.h
 */

#include <unistd.h>
#include <stdio.h>
#include "qassert.h"
#include "qactor.h"
#include "qconfig.h"
#include "qengine.h"
#include "qdefines.h"
#include "qlist.h"
#include "qlog.h"
#include "qluautil.h"
#include "qmailbox.h"
#include "qmalloc.h"
#include "qserver.h"
#include "qthread.h"
#include "qthread_log.h"

static void server_box_func(int fd, int flags, void *data) {
  UNUSED(fd);
  UNUSED(flags);
  qinfo("server box");
  qlist_t *list;
  qthread_t *thread = (qthread_t*)data;
  qmailbox_get(thread->in_box[0], &list);
  qlist_t *pos, *next;
  for (pos = list->next; pos != list; ) {
    qmsg_t *msg = qlist_entry(pos, qmsg_t, entry);
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
    qinfo("handle %d msg", msg->type);
    (g_thread_msg_handlers[msg->type])(thread, msg);

next:
    qfree(msg);
    pos = next;
  }
}

static void thread_box_func(int fd, int flags, void *data) {
  qinfo("in thread_box_func");
  UNUSED(fd);
  UNUSED(flags);
  qlist_t *list;
  qthread_box_t *thread_box = (qthread_box_t*)data;
  qthread_t *thread = thread_box->thread;
  qmailbox_t *box = thread_box->box;
  qmailbox_get(box, &list);
  qlist_t *pos, *next;
  for (pos = list->next; pos != list; ) {
    qmsg_t *msg = qlist_entry(pos, qmsg_t, entry);
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
    qinfo("handle %d msg", msg->type);
    (g_thread_msg_handlers[msg->type])(thread, msg);

next:
    qfree(msg);
    pos = next;
  }
}

static void* main_loop(void *arg) {
  qthread_t *thread = (qthread_t*)arg;
  g_server->thread_log[thread->tid] = qthread_log_init(thread->engine, thread->tid);
  thread->started = 1;
  while (thread->started && qengine_loop(thread->engine) == 0) {
  }
  return NULL;
}

qthread_t* qthread_new(struct qserver_t *server, qtid_t tid) {
  int thread_num = server->config->thread_num;
  qthread_t *thread = qalloc_type(qthread_t);
  int i;
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
  thread->tid = tid;

  thread->thread_box = (qthread_box_t**)qmalloc((thread_num + 1) * sizeof(qthread_box_t*));
  thread->in_box  = (qmailbox_t**)qmalloc((thread_num + 1) * sizeof(qmailbox_t*));
  thread->out_box = (qmailbox_t**)qmalloc((thread_num + 1) * sizeof(qmailbox_t*));
  for (i = 0; i < thread_num + 1; ++i) {
    thread->out_box[i] = NULL;
    if (i == (int)tid) {
      thread->in_box[i] = NULL;
      continue;
    }
    if (i == 0) {
      thread->in_box[i] = qmailbox_new(server_box_func, thread);
    } else {
      thread->thread_box[i] = qalloc_type(qthread_box_t);
      thread->in_box[i] = qmailbox_new(thread_box_func, thread->thread_box[i]);
      thread->thread_box[i]->thread = thread;
      thread->thread_box[i]->box = thread->in_box[i];
    }
    qmailbox_active(thread->engine, thread->in_box[i]);
  }

  thread->state = qlua_new_state();
  qlist_entry_init(&(thread->actor_list));
  int result;
  result = pthread_create(&thread->id, NULL, main_loop, thread);
  qassert(result == 0);
  /* ugly, but works */
  while (thread->started == 0) {
    usleep(100);
  }
  return thread;
}
