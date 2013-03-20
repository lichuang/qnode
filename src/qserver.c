/*
 * See Copyright Notice in qnode.h
 */

#include <signal.h>
#include <stdio.h>
#include "qactor.h"
#include "qassert.h"
#include "qdescriptor.h"
#include "qengine.h"
#include "qdefines.h"
#include "qlog.h"
#include "qlog_thread.h"
#include "qmailbox.h"
#include "qmempool.h"
#include "qmsg.h"
#include "qnet.h"
#include "qserver.h"
#include "qthread.h"
#include "qthread_log.h"

extern qserver_msg_handler g_server_msg_handlers[];

struct qserver_t *g_server;

static void
server_accept(int fd, int flags, void *data) {
  UNUSED(fd);
  UNUSED(flags);
  UNUSED(data);
  qinfo("add a socket....");
}

static int
init_server_event(struct qserver_t *server) {
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

static void
server_box(int fd, int flags, void *data) {
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
    msg->handled = 1;
    qmsg_destroy(msg);
    pos = next;
  }
}

qtid_t
qserver_worker_thread() {
  static qtid_t i = 1;
  i = (i + 1) % g_server->config->thread_num + 1;
  return i;
}

qactor_t*
qserver_get_actor(qid_t id) {
  return g_server->actors[id];
}

static void
server_start(qserver_t *server) {
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

static int
init_worker_threads(qserver_t *server) {
  int i, j;
  qconfig_t *config = server->config;
  int thread_num = config->thread_num;
  qmem_pool_t *pool = server->pool;
  
  server->threads = qalloc(pool, thread_num * sizeof(qthread_t*));
  if (server->threads == NULL) {
    goto error;
  }
  server->threads[0] = NULL;

  server->in_box = qalloc(pool, thread_num * sizeof(qmailbox_t*));
  if (server->in_box == NULL) {
    goto error;
  }
  server->in_box[0] = NULL;

  server->thread_log = qalloc(pool, thread_num * sizeof(qthread_log_t*));
  if (server->thread_log == NULL) {
    goto error;
  }

  server->out_box = qalloc(pool, thread_num * sizeof(qmailbox_t*));
  if (server->out_box == NULL) {
    goto error;
  }
  server->out_box[0] = NULL;

  for (i = 1; i <= thread_num; ++i) {
    qmailbox_t *box = qmailbox_new(pool, server_box, NULL);
    if (box == NULL) {
      goto error;
    }
    box->reader = box;
    qmailbox_active(server->engine, box);
    server->in_box[i] = box;
    server->threads[i] = qthread_new(server, i); 
    if (server->threads[i] == NULL) {
      goto error;
    }
    server->out_box[i] = server->threads[i]->in_box[0];
    server->threads[i]->out_box[0] = box;
  }

  for (i = 1; i <= thread_num; ++i) {
    qthread_t *thread1 = server->threads[i];
    for (j = 1; j <= thread_num; ++j) {
      if (j == i) {
        continue;
      }
      qthread_t *thread2 = server->threads[j];

      thread1->out_box[j] = thread2->in_box[i];
      thread2->out_box[i] = thread1->in_box[j];
    }
  }
  return 0;

error:
  /*
   * no need to free memory, cause this function called
   * when server start, just end the server wiil be fine
   */
  return -1;
}

static void
sig_handler(int sig) {
  qinfo("caught signal %d", sig);
  switch (sig) {
  case SIGTERM:
  case SIGINT:
  case SIGQUIT:
  case SIGABRT:
    g_server->status = STOPPING;
    break;
  default:
    break;
  }
}

static void
setup_signal() {
  struct sigaction act;

  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_NODEFER | SA_ONSTACK | SA_RESETHAND;
  act.sa_handler = sig_handler;
  sigaction(SIGTERM, &act, NULL);
  sigaction(SIGINT,  &act, NULL);
  sigaction(SIGQUIT, &act, NULL);
  sigaction(SIGABRT, &act, NULL);
}

static int
server_init(qmem_pool_t *pool, struct qconfig_t *config) {
  qassert(config);
  qassert(config->thread_num > 0);
  qassert(g_server == NULL);

  qserver_t *server = qcalloc(pool, sizeof(qserver_t));
  if (server == NULL) {
    goto error;
  }
  g_server = server;
  g_server->pool = pool;
  if (qlog_thread_new(pool, config->thread_num + 1) < 0) {
    goto error;
  }
  server->config = config;
  server->engine = qengine_new(g_server->pool);
  if (init_server_event(server) < 0) {
    goto error;
  }
  server->actors = (qactor_t**)qalloc(pool, QID_MAX * sizeof(qactor_t*));
  if (server->actors == NULL) {
    goto error;
  }

  server->descriptors = (qdescriptor_t**)qcalloc(pool, QID_MAX * sizeof(qdescriptor_t*));
  if (server->descriptors == NULL) {
    goto error;
  }

  if (init_worker_threads(server) < 0) {
    goto error;
  }
  qidmap_init(&server->id_map);
  qmutex_init(&server->id_map_mutex);
  setup_signal();

  server->thread_log[0] = qthread_log_init(server->engine, 0);
  server_start(server);
  qinfo("qserver status...");
  return 0;

error:
  return -1;
}

static void
destroy_threads() {
  int i;
  for (i = 1; i <= g_server->config->thread_num; ++i) {
    qthread_t *thread = g_server->threads[i];
    qthread_destroy(thread);
  }
  qlog_thread_destroy();
}

static void
destroy_server() {
  if (g_server->status == STOPPED) {
    return;
  }
  g_server->status = STOPPED;
  qinfo("destroy_server");
  destroy_threads();
  qmem_pool_destroy(g_server->pool);
}

int
qserver_run(struct qconfig_t *config) {
  qmem_pool_t *pool = qmem_pool_create();
  if (pool == NULL) {
    return -1;
  }
  if (server_init(pool, config) != 0) {
    return -1;
  }
  g_server->status = RUNNING;
  while (g_server->status == RUNNING && qengine_loop(g_server->engine) == 0) {
  }
  destroy_server();
  return 0;
}

void
qserver_new_actor(struct qactor_t *actor) {
  qassert(g_server->actors[actor->aid] == NULL);
  g_server->actors[actor->aid] = actor;
  g_server->num_actor++;
}
