/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qactor.h"
#include "qassert.h"
#include "qdescriptor.h"
#include "qengine.h"
#include "qdefines.h"
#include "qlog.h"
#include "qlog_thread.h"
#include "qmalloc.h"
#include "qmailbox.h"
#include "qmsg.h"
#include "qnet.h"
#include "qserver.h"
#include "qthread.h"
#include "qthread_log.h"

extern qserver_msg_handler g_server_msg_handlers[];

struct qserver_t *g_server;

static void server_accept(int fd, int flags, void *data) {
  UNUSED(fd);
  UNUSED(flags);
  UNUSED(data);
  qinfo("add a socket....");
}

static int init_server_event(struct qserver_t *server) {
  return 0;
  int fd = qnet_tcp_listen(22222, "127.0.0.1");
  if (fd < 0) {
    return -1;
  }

  qengine_t *engine = server->engine;
  if (qengine_add_event(engine, fd, QEVENT_READ, server_accept, server) < 0) {
    return -1;
  }
  return 0;
}

static void server_box(int fd, int flags, void *data) {
  UNUSED(fd);
  UNUSED(flags);
  qinfo("handle server msg");
  qlist_t *list;
  qmailbox_t *box = (qmailbox_t*)data;
  qmailbox_get(box, &list);
  qlist_t *pos, *next;
  for (pos = list->next; pos != list; ) {
    qmsg_t *msg = qlist_entry(pos, qmsg_t, entry);
    next = pos->next;
    qlist_del_init(&(msg->entry));
    if (msg == NULL) {
      goto next;
    }
    if (!qmsg_is_wmsg(msg)) {
      qerror("msg %d is not worker msg", msg->type);
      goto next;
    }
    if (qmsg_invalid_type(msg->type)) {
      qerror("msg %d is not valid msg type", msg->type);
      goto next;
    }
    qinfo("handle %d msg", msg->type);
    (g_server_msg_handlers[msg->type])(g_server, msg);

next:
    qfree(msg);
    pos = next;
  }
}

qtid_t qserver_worker_thread() {
  static qtid_t i = 1;
  i = (i + 1) % g_server->config->thread_num + 1;
  return i;
}

qactor_t* qserver_get_actor(qid_t id) {
  return g_server->actors[id];
}

static void server_start(qserver_t *server) {
  UNUSED(server);
  qid_t aid = qactor_new_id();
  qassert(aid != QID_INVALID);
  qactor_t *actor = qactor_new(aid);
  qtid_t tid = qserver_worker_thread();
  qmsg_t *msg = qmsg_new(QSERVER_THREAD_TID, tid);
  if (msg == NULL) {
    return;
  }
  qmsg_init_sstart(msg, actor);
  qmsg_send(msg);
}

static void init_thread(qserver_t *server) {
  int i, j;
  qconfig_t *config = server->config;
  server->threads = (qthread_t**)qmalloc(config->thread_num * sizeof(qthread_t*));
  qalloc_assert(server->threads);
  server->threads[0] = NULL;
  server->in_box = (qmailbox_t**)qmalloc(config->thread_num * sizeof(qmailbox_t*));
  qalloc_assert(server->in_box);
  server->in_box[0] = NULL;
  server->thread_log = (qthread_log_t**)qmalloc(config->thread_num * sizeof(qthread_t*));
  qalloc_assert(server->thread_log);
  server->out_box = (qmailbox_t**)qmalloc(config->thread_num * sizeof(qmailbox_t*));
  qalloc_assert(server->out_box);
  server->out_box[0] = NULL;
  for (i = 1; i <= config->thread_num; ++i) {
    qmailbox_t *box = qmailbox_new(server_box, NULL);
    box->reader = box;
    qmailbox_active(server->engine, box);
    server->in_box[i] = box;
    server->threads[i] = qthread_new(server, i); 
    qassert(server->threads[i]);
    server->out_box[i] = server->threads[i]->in_box[0];
    server->threads[i]->out_box[0] = box;
  }

  for (i = 1; i <= config->thread_num; ++i) {
    qthread_t *thread1 = server->threads[i];
    for (j = 1; j <= config->thread_num; ++j) {
      if (j == i) {
        continue;
      }
      qthread_t *thread2 = server->threads[j];

      thread1->out_box[j] = thread2->in_box[i];
      thread2->out_box[i] = thread1->in_box[j];
    }
  }
}

static int server_init(struct qconfig_t *config) {
  qassert(config);
  qassert(config->thread_num > 0);
  qassert(g_server == NULL);
  int i;
  qlog_thread_new(config->thread_num + 1);
  qserver_t *server = qalloc_type(qserver_t);
  g_server = server;
  server->config = config;
  server->engine = qengine_new();
  if (init_server_event(server) < 0) {
    qengine_destroy(server->engine);
    qfree(server);
    return -1;
  }
  server->actors = (qactor_t**)qmalloc(QID_MAX * sizeof(qactor_t*));
  if (server->actors == NULL) {
    qengine_destroy(server->engine);
    qfree(server);
    return -1;
  }

  server->descriptors = (qdescriptor_t**)qmalloc(QID_MAX * sizeof(qdescriptor_t*));
  for (i = 0; i < QID_MAX; ++i) {
    server->descriptors[i] = NULL;
  }

  init_thread(server);
  qidmap_init(&server->id_map);
  qmutex_init(&server->id_map_mutex);

  server->thread_log[0] = qthread_log_init(server->engine, 0);
  server_start(server);
  qinfo("qserver started...");
  return 0;
}

int qserver_run(struct qconfig_t *config) {
  if (server_init(config) != 0) {
    return -1;
  }
  qengine_loop(g_server->engine);
  return 0;
}

void qserver_new_actor(struct qactor_t *actor) {
  qassert(g_server->actors[actor->aid] == NULL);
  g_server->actors[actor->aid] = actor;
  g_server->num_actor++;
}
