/*
 * See Copyright Notice in qnode.h
 */

#include <signal.h>
#include <stdio.h>
#include "qalloc.h"
#include "qactor.h"
#include "qassert.h"
#include "qdescriptor.h"
#include "qengine.h"
#include "qdefines.h"
#include "qlog.h"
#include "qlog_thread.h"
#include "qmailbox.h"
#include "qmsg.h"
#include "qnet.h"
#include "qserver.h"
#include "qthread.h"
#include "qthread_log.h"

extern qserver_msg_handler g_server_msg_handlers[];

qserver_t *g_server;

/* thread init condition */
static int          init_thread_count;
static qcond_t      init_thread_cond;
static qmutex_t     init_thread_lock; 

/* server running flag */
static volatile int running;

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
  qmsg_t      *msg;
  qlist_t     *list;
  qmailbox_t  *box;
  qlist_t     *pos, *next;

  UNUSED(fd);
  UNUSED(flags);

  box = (qmailbox_t*)data;
  qmailbox_get(box, &list);
  for (pos = list->next; pos != list; ) {
    msg = qlist_entry(pos, qmsg_t, entry);
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

qtid_t qserver_worker_thread() {
  static qtid_t i;

  i = 1;
  i = (i + 1) % g_server->config->thread_num + 1;
  return i;
}

qactor_t* qserver_get_actor(qid_t id) {
  return g_server->actors[id];
}

static void server_start(qserver_t *server) {
  qid_t   aid;
  qtid_t  tid;
  qmsg_t *msg;

  UNUSED(server);
  aid = qactor_new_id();
  qassert(aid != QID_INVALID);
  tid = qserver_worker_thread();
  msg = qmsg_new(QMAIN_THREAD_TID, tid);
  if (msg == NULL) {
    return;
  }
  qmsg_init_sstart(msg, aid);
  qmsg_send(msg);
}

static void wait_threads(int thread_num) {
  qmutex_lock(&init_thread_lock);
  while (init_thread_count < thread_num) {
    qcond_wait(&init_thread_cond, &init_thread_lock);
  }
  qmutex_unlock(&init_thread_lock);
}

static int init_worker_threads(qserver_t *server) {
  int           i, j;
  int           thread_num;
  qconfig_t    *config;
  qmailbox_t   *box;
  qthread_t    *thread1, *thread2;
  
  config = server->config;
  thread_num = config->thread_num;

  server->threads = qalloc(thread_num * sizeof(qthread_t*));
  if (server->threads == NULL) {
    goto error;
  }
  server->threads[0] = NULL;

  server->in_box = qalloc(thread_num * sizeof(qmailbox_t*));
  if (server->in_box == NULL) {
    goto error;
  }
  server->in_box[0] = NULL;

  server->thread_log = qalloc(thread_num * sizeof(qthread_log_t*));
  if (server->thread_log == NULL) {
    goto error;
  }

  server->out_box = qalloc(thread_num * sizeof(qmailbox_t*));
  if (server->out_box == NULL) {
    goto error;
  }
  server->out_box[0] = NULL;

  init_thread_count = 0;

  /* create worker thread and mailbox */
  for (i = 1; i <= thread_num; ++i) {
    box = qmailbox_new(server_box, NULL);
    if (box == NULL) {
      goto error;
    }
    box->reader = box;
    /* add the worker-main box into main thread event engine */
    qmailbox_active(server->engine, box);
    server->in_box[i] = box;
    server->threads[i] = qthread_new(server, i); 
    if (server->threads[i] == NULL) {
      goto error;
    }
    server->out_box[i] = server->threads[i]->in_box[0];
    server->threads[i]->out_box[0] = box;
  }
    
  /* wait for the worker threads start */
  wait_threads(thread_num);

  qmutex_destroy(&init_thread_lock);
  qcond_destroy(&init_thread_cond);

  /* link worker thread mailbox */
  for (i = 1; i <= thread_num; ++i) {
    thread1 = server->threads[i];
    for (j = 1; j <= thread_num; ++j) {
      if (j == i) {
        continue;
      }
      thread2 = server->threads[j];

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

static void sig_handler(int sig) {
  qinfo("caught signal %d", sig);
  switch (sig) {
  case SIGTERM:
  case SIGINT:
  case SIGQUIT:
  case SIGABRT:
    running = 0;
    break;
  default:
    break;
  }
}

static void setup_signal() {
  struct sigaction act;

  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_NODEFER | SA_ONSTACK | SA_RESETHAND;
  act.sa_handler = sig_handler;
  sigaction(SIGTERM, &act, NULL);
  sigaction(SIGINT,  &act, NULL);
  sigaction(SIGQUIT, &act, NULL);
  sigaction(SIGABRT, &act, NULL);
}

static int server_init(qconfig_t *config) {
  qserver_t   *server;

  qassert(config);
  qassert(config->thread_num > 0);
  qassert(g_server == NULL);

  server = qcalloc(sizeof(qserver_t));

  qmutex_init(&init_thread_lock);
  qcond_init(&init_thread_cond);

  if (server == NULL) {
    goto error;
  }
  g_server = server;

  init_thread_count = 0;
  if (qlog_thread_new(config->thread_num + 1) < 0) {
    goto error;
  }
  /* wait for the log thread start */
  wait_threads(1);

  server->config = config;
  server->engine = qengine_new();
  if (init_server_event(server) < 0) {
    goto error;
  }
  server->actors = qalloc(QID_MAX * sizeof(qactor_t*));
  if (server->actors == NULL) {
    goto error;
  }

  server->descriptors = qcalloc(QID_MAX * sizeof(qdescriptor_t*));
  if (server->descriptors == NULL) {
    goto error;
  }

  if (init_worker_threads(server) < 0) {
    goto error;
  }
  qidmap_init(&server->id_map);
  qmutex_init(&server->id_map_mutex);
  setup_signal();

  server->thread_log[0] = qthread_log_init(0);
  server_start(server);

  return 0;

error:
  return -1;
}

static void destroy_threads() {
  int         i;
  qthread_t  *thread;

  for (i = 1; i <= g_server->config->thread_num; ++i) {
    thread = g_server->threads[i];
    qthread_destroy(thread);
  }
  qlog_thread_destroy();
}

static void destroy_server() {
  destroy_threads();
}

int qserver_run(qconfig_t *config) {
  if (server_init(config) != 0) {
    return -1;
  }
  running = 1;
  while (running && qengine_loop(g_server->engine) == 0) {
  }
  destroy_server();
  return 0;
}

void qserver_new_actor(qactor_t *actor) {
  qassert(g_server->actors[actor->aid] == NULL);
  g_server->actors[actor->aid] = actor;
  g_server->num_actor++;
}

void qserver_worker_started() {
  qmutex_lock(&init_thread_lock);
  init_thread_count++;
  qcond_signal(&init_thread_cond);
  qmutex_unlock(&init_thread_lock);
}
