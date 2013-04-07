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

static void server_box_func(int fd, int flags, void *data) {
  UNUSED(fd);
  UNUSED(flags);
  qinfo("server box");

  qmsg_t      *msg;
  qlist_t     *list;
  qlist_t     *pos, *next;
  qthread_t   *thread;

  thread = (qthread_t*)data;
  qmailbox_get(thread->in_box[0], &list);
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
    qinfo("handle %d msg", msg->type);
    (g_thread_msg_handlers[msg->type])(thread, msg);

next:
    msg->handled = 1;
    qmsg_destroy(msg);
    pos = next;
  }
}

static void thread_box_func(int fd, int flags, void *data) {
  qinfo("in thread_box_func");
  UNUSED(fd);
  UNUSED(flags);

  qlist_t       *list, *pos, *next;
  qmailbox_t    *box;
  qmsg_t        *msg;
  qthread_box_t *thread_box;
  qthread_t     *thread;

  thread_box = (qthread_box_t*)data;
  thread = thread_box->thread;
  box = thread_box->box;
  qmailbox_get(box, &list);

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
    qinfo("handle %d msg", msg->type);
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
  g_server->thread_log[thread->tid] = qthread_log_init(thread->engine,
                                                       thread->tid);
  thread->running = 1;
  qserver_worker_started();
  while (thread->running && qengine_loop(thread->engine) == 0) {
  }
  return NULL;
}

qthread_t* qthread_new(struct qserver_t *server, qtid_t tid) {
  int           i, thread_num;
  qthread_t    *thread;

  thread_num = server->config->thread_num;
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

  thread->thread_box = qcalloc((thread_num + 1) * sizeof(qthread_box_t*));
  if (thread->thread_box == NULL) {
    return NULL;
  }
  thread->in_box  = qcalloc((thread_num + 1) * sizeof(qmailbox_t*));
  if (thread->in_box == NULL) {
    return NULL;
  }
  thread->out_box = qcalloc((thread_num + 1) * sizeof(qmailbox_t*));
  if (thread->out_box == NULL) {
    return NULL;
  }
  for (i = 0; i < thread_num + 1; ++i) {
    thread->out_box[i] = NULL;
    if (i == (int)tid) {
      thread->in_box[i] = NULL;
      continue;
    }
    if (i == 0) {
      /* communicate with main thread */
      thread->in_box[i] = qmailbox_new(server_box_func, thread);
      qassert(thread->in_box[i]->signal);
    } else {
      /* communicate with other worker thread */
      thread->thread_box[i] = qcalloc(sizeof(qthread_box_t));
      thread->in_box[i] = qmailbox_new(thread_box_func, thread->thread_box[i]);

      thread->thread_box[i]->thread = thread;
      qassert((char*)(thread->in_box[i]) != (char*)thread);

      thread->thread_box[i]->box = thread->in_box[i];
      qassert(thread->in_box[i]->signal);
    }
    if (thread->in_box[i] == NULL) {
      return NULL;
    }
    /* add the in box in the worker event engine */
    qmailbox_active(thread->engine, thread->in_box[i]);
  }

  /* create the lua VM for the thread */
  thread->state = qlua_new_state();
  /* init the actor list */
  qlist_entry_init(&(thread->actor_list));
  pthread_create(&thread->id, NULL,
                 worker_thread_main_loop, thread);

  return thread;
}

void qthread_destroy(qthread_t *thread) {
  qmsg_t *msg;

  /* send stop message to thread */
  msg = qmsg_new(0, thread->tid);
  qmsg_init_sstop(msg);
  qmsg_send(msg);

  /* wait for the thread stop */
  pthread_join(thread->id, NULL);
}
