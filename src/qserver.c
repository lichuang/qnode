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
#include "qlogger.h"
#include "qmailbox.h"
#include "qmsg.h"
#include "qnet.h"
#include "qserver.h"
#include "qsignal.h"
#include "qworker.h"
#include "qthread_log.h"

extern qmsg_func_t* g_server_msg_handlers[];

qserver_t    *g_server;
volatile int  g_quit = 0;

/* thread init condition */
static int          init_thread_count;
static qcond_t      init_thread_cond;
static qmutex_t     init_thread_lock; 

static void server_accept(int fd, int flags, void *data);
static int  init_server_event(qserver_t *server);
static int  server_msg_handler(qmsg_t *msg, void *reader);
static void server_start(qserver_t *server);
static void wait_threads(int thread_num);
static int  init_worker_threads(qserver_t *server);
static void signal_handler(int sig);
static void setup_signal();
static int  server_init(qconfig_t *config);
static void destroy_threads();
static void destroy_server();

static void
server_accept(int fd, int flags, void *data) {
  UNUSED(fd);
  UNUSED(flags);
  UNUSED(data);
  qinfo("add a socket....");
}

static int
init_server_event(qserver_t *server) {
  return 0;
  int fd = qnet_tcp_listen(22222, "127.0.0.1");
  if (fd < 0) {
    return -1;
  }

  qengine_t *engine = server->engine;
  if (qengine_add_event(engine, fd, QEVENT_READ,
                        server_accept, server) < 0) {
    return -1;
  }
  return 0;
}

static int
server_msg_handler(qmsg_t *msg, void *reader) {
  return (*g_server_msg_handlers[msg->type])(msg, reader);
}

qtid_t
qserver_worker() {
  static qtid_t i;

  i = 1;
  i = (i + 1) % g_server->config->thread_num + 1;
  return i;
}

qactor_t*
qserver_get_actor(qid_t id) {
  return g_server->actors[id];
}

static void
server_start(qserver_t *server) {
  qid_t   aid;
  qtid_t  tid;
  qmsg_t *msg;

  UNUSED(server);
  aid = qactor_new_id();
  qassert(aid != QID_INVALID);
  tid = qserver_worker();
  msg = qmsg_new(QMAIN_THREAD_TID, tid);
  if (msg == NULL) {
    return;
  }
  qmsg_init_start(msg, aid);
  qworker_send(tid, msg);
}

static void
wait_threads(int thread_num) {
  qmutex_lock(&init_thread_lock);
  while (init_thread_count < thread_num) {
    qcond_wait(&init_thread_cond, &init_thread_lock);
  }
  qmutex_unlock(&init_thread_lock);
}

static int
init_worker_threads(qserver_t *server) {
  int           i;
  int           thread_num;
  qconfig_t    *config;
  
  config = server->config;
  thread_num = config->thread_num;

  server->workers = qalloc(thread_num * sizeof(qworker_t*));
  if (server->workers == NULL) {
    goto error;
  }
  server->workers[0] = NULL;

  server->thread_log = qalloc(thread_num * sizeof(qthread_log_t*));
  if (server->thread_log == NULL) {
    goto error;
  }

  init_thread_count = 0;

  /* create worker threads */
  for (i = 1; i <= thread_num; ++i) {
    server->workers[i] = qworker_new(i); 
    if (server->workers[i] == NULL) {
      goto error;
    }
  }
    
  /* wait for the worker threads start */
  wait_threads(thread_num);

  qmutex_destroy(&init_thread_lock);
  qcond_destroy(&init_thread_cond);

  return 0;

error:
  /*
   * no need to free memory, cause this function called
   * when server start, just end the server wiil be fine
   */
  return -1;
}

static void
signal_handler(int sig) {
  qmsg_t     *msg;
  qmailbox_t *box;

  msg = qmsg_new(0, 0);
  if (msg == NULL) {
    return;
  }

  qmsg_init_signal(msg, sig);
  box = &(g_server->box);
  qmailbox_add(box, msg);
}

static void
setup_signal() {
  struct sigaction act;

  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_NODEFER | SA_ONSTACK | SA_RESETHAND;
  act.sa_handler = signal_handler;
  sigaction(SIGTERM, &act, NULL);
  sigaction(SIGINT,  &act, NULL);
  sigaction(SIGQUIT, &act, NULL);
  sigaction(SIGABRT, &act, NULL);
}

static int
server_init(qconfig_t *config) {
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
  if (qlogger_new(config->thread_num + 1) < 0) {
    goto error;
  }
  /* wait for the log thread start */
  wait_threads(1);

  server->config = config;
  server->engine = qengine_new();
  if (init_server_event(server) < 0) {
    goto error;
  }
  qmailbox_init(&(server->box), server_msg_handler,
                server->engine, server);
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

static void
destroy_threads() {
  int         i;
  qworker_t  *worker;

  for (i = 1; i <= g_server->config->thread_num; ++i) {
    worker = g_server->workers[i];
    qworker_destroy(worker);
  }
  qlogger_destroy();
}

static void
destroy_server() {
  destroy_threads();
}

int
qserver_run(qconfig_t *config) {
  if (server_init(config) != 0) {
    return -1;
  }
  qengine_loop(g_server->engine);
  destroy_server();
  return 0;
}

void
qserver_new_actor(qactor_t *actor) {
  qassert(g_server->actors[actor->aid] == NULL);
  g_server->actors[actor->aid] = actor;
  g_server->num_actor++;
}

void
qserver_worker_started() {
  qmutex_lock(&init_thread_lock);
  init_thread_count++;
  qcond_signal(&init_thread_cond);
  qmutex_unlock(&init_thread_lock);
}
