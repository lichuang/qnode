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
#include "qmsg.h"
#include "qserver.h"
#include "qthread.h"
#include "qthread_log.h"

extern smsg_handler smsg_handlers[];

static void server_box_func(int fd, int flags, void *data) {
  UNUSED(fd);
  UNUSED(flags);
  qinfo("thread box");
  qlist_t *list;
  qthread_t *thread = (qthread_t*)data;
  qmailbox_get(thread->box, &list);
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
    (smsg_handlers[msg->type])(thread, msg);

next:
    qfree(msg);
    pos = next;
  }
}

static void thread_box_func(int fd, int flags, void *data) {
  UNUSED(fd);
  UNUSED(flags);
  UNUSED(data);
}

static void* main_loop(void *arg) {
  qthread_t *thread = (qthread_t*)arg;
  //qmsg_t *msg = qmsg_new(thread->tid, QSERVER_THREAD_TID);
  //qmsg_init_thread_start(msg);
  g_server->thread_log[thread->tid] = qthread_log_init(thread->engine, thread->tid);
  thread->started = 1;
  qengine_loop(thread->engine);
  return NULL;
}

qthread_t* qthread_new(struct qserver_t *server, qtid_t tid) {
  int thread_num = server->config->thread_num;
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
  thread->tid = tid;
  thread->box = qmailbox_new(server_box_func, thread);
  qassert(thread->box);
  qlist_entry_init(&(thread->actor_list));
  thread->channels = (qchannel_t**)qmalloc((thread_num + 1)* sizeof(qchannel_t*));
  thread->channels[0] = NULL;
  int i;
  for (i = 1; i < thread_num + 1; ++i) {
    qchannel_t *channel = qalloc_type(qchannel_t);
    channel->in_box = channel->out_box = NULL;
    thread->channels[i] = channel;
  }
  int result;
  result = pthread_create(&thread->id, NULL, main_loop, thread);
  qassert(result == 0);
  /* ugly, but works */
  while (thread->started == 0) {
    usleep(100);
  }
  qmailbox_active(thread->engine, thread->box);
  return thread;
}

struct qmailbox_t* qthread_mailbox(qthread_t *thread) {
  return thread->box;
}

void qthread_init_thread_channel(qthread_t *src_thread, qthread_t *dst_thread) {
  qtid_t src_id = src_thread->tid;
  qtid_t dst_id = dst_thread->tid;
  qchannel_t *src_channel = src_thread->channels[dst_id];
  qchannel_t *dst_channel = dst_thread->channels[src_id];
  qassert(src_channel->out_box == NULL);
  qassert(dst_channel->in_box  == NULL);
  qmailbox_t *box = qmailbox_new(thread_box_func, dst_thread);
  src_channel->out_box = box;
  dst_channel->in_box  = box;
}

