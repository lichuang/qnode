/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qactor.h"
#include "qassert.h"
#include "qconfig.h"
#include "qengine.h"
#include "qlog.h"
#include "qmalloc.h"
#include "qmailbox.h"
#include "qmsg.h"
#include "qnet.h"
#include "qserver.h"
#include "qthread.h"
#include "qthread_log.h"

extern wmsg_handler wmsg_handlers[];

struct qserver_t *g_server;

static void server_accept(int fd, int flags, void *data) {
  qinfo("add a connection....");
}

static int init_server_event(struct qserver_t *server) {
  return 0;
  int fd = qnet_tcp_server(22222, "127.0.0.1");
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
    (wmsg_handlers[msg->type])(g_server, msg);

next:
    if (!qmsg_undelete(msg)) {
      qfree(msg);
    }
    pos = next;
  }
}

void qserver_send_mail(struct qmsg_t *msg) {
  qmailbox_add(g_server->thread_box[1], msg);
}

int qserver_add_mail(qtid_t tid, struct qmsg_t *msg) {
  qmailbox_add(g_server->box[tid], msg);
  return 0;
}

static void server_start(qserver_t *server) {
  qid_t aid = qactor_new_id();
  qassert(aid != QAID_INVALID);
  qactor_t *actor = qactor_new(aid);
  qmsg_t *msg = qmsg_new();
  if (msg == NULL) {
    return;
  }
  qmsg_init_sstart(msg, actor);
  qserver_send_mail(msg);
  qinfo("add a msg, type: %d, flag: %d", msg->type, msg->flag);
}

static int server_init(struct qconfig_t *config) {
  qassert(config);
  qassert(config->thread_num > 0);
  qassert(g_server == NULL);
  qlog_thread_new(config->thread_num + 1);
  qserver_t *server = qalloc_type(qserver_t);
  server->config = config;
  server->engine = qengine_new();
  if (init_server_event(server) < 0) {
    qengine_destroy(server->engine);
    qfree(server);
    return -1;
  }
  server->actors = (qactor_t**)qmalloc(QID_MAX * sizeof(qthread_t*));
  if (server->actors == NULL) {
    qengine_destroy(server->engine);
    qfree(server);
    return -1;
  }
  /* alloc threads and mailbox */
  server->threads = (qthread_t**)qmalloc(config->thread_num * sizeof(qthread_t*));
  qalloc_assert(server->threads);
  server->threads[0] = NULL;
  server->box = (qmailbox_t**)qmalloc(config->thread_num * sizeof(qmailbox_t*));
  qalloc_assert(server->box);
  server->box[0] = NULL;
  server->thread_log = (qthread_log_t**)qmalloc(config->thread_num * sizeof(qthread_t*));
  qalloc_assert(server->thread_log);
  server->thread_box = (qmailbox_t**)qmalloc(config->thread_num * sizeof(qmailbox_t*));
  qalloc_assert(server->thread_box);
  server->thread_box[0] = NULL;
  int i;
  for (i = 1; i <= config->thread_num; ++i) {
    server->box[i] = qmailbox_new(server_box, NULL);
    server->box[i]->reader = server->box[i];
    qassert(server->box[i]);
    qmailbox_active(server->engine, server->box[i]);
    server->threads[i] = qthread_new(server, i); 
    qassert(server->threads[i]);
    server->thread_box[i] = qthread_mailbox(server->threads[i]);
  }
  qidmap_init(&server->id_map);
  qmutex_init(&server->id_map_mutex);
  g_server = server;

  server->thread_log[0] = qthread_log_init(server->engine);
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
