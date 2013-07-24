/*
 * See Copyright Notice in qnode.h
 */

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include "qalloc.h"
#include "qactor.h"
#include "qassert.h"
#include "qconfig.h"
#include "qdescriptor.h"
#include "qengine.h"
#include "qdefines.h"
#include "qlog.h"
#include "qlogger.h"
#include "qmailbox.h"
#include "qmsg.h"
#include "qmmsg.h"
#include "qnet.h"
#include "qserver.h"
#include "qsignal.h"
#include "qworker.h"
#include "qthread_log.h"
#include "qwmsg.h"

extern qmsg_func_t* server_msg_handlers[];

static qengine_t *engine;
static qmailbox_t box;
qserver_t        *server;

static void server_accept(int fd, int flags, void *data);
static int  init_server_event(qserver_t *server);
static int  server_msg_handler(qmsg_t *msg, void *reader);
static void server_start(qserver_t *server);
static int  init_workers(qserver_t *server);
static void signal_handler(int sig);
static void setup_signal();
static int  server_init();
static void destroy_threads();
static void destroy_server();
static void make_daemon();
static void save_pid();
static int  set_core_size();

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

  if (qengine_add_event(engine, fd, QEVENT_READ,
                        server_accept, server) < 0) {
    return -1;
  }
  return 0;
}

static int
server_msg_handler(qmsg_t *msg, void *reader) {
  qinfo("main handle %d msg", msg->type);
  return (*server_msg_handlers[msg->type])(msg, reader);
}

qid_t
qserver_worker() {
  static qid_t i = 1;

  i = ((i + 1) % config.worker) + 1;
  return i;
}

static void
server_start(qserver_t *server) {
  qid_t  tid;
  qmsg_t *msg;

  UNUSED(server);
  tid = qserver_worker();
  msg = qwmsg_start_new(QMAINTHREAD_TID, tid);
  if (msg == NULL) {
    return;
  }
  qworker_send(msg);
}

static int
init_workers(qserver_t *server) {
  int           i;
  int           worker;
  
  worker = config.worker + 1;
  workers[0] = NULL;

  /* create worker threads */
  for (i = 1; i < worker; ++i) {
    workers[i] = qworker_new(i); 
    if (workers[i] == NULL) {
      goto error;
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
signal_handler(int signo) {
  qmsg_t     *msg;

  msg = qmmsg_signal_new(signo);
  if (msg == NULL) {
    return;
  }

  qmailbox_add(&box, msg);
}

static void
setup_signal() {
  struct sigaction act;

  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_NODEFER;
  act.sa_handler = signal_handler;
  sigaction(SIGTERM, &act, NULL);
  sigaction(SIGINT,  &act, NULL);
  sigaction(SIGQUIT, &act, NULL);
  sigaction(SIGABRT, &act, NULL);
  sigaction(SIGUSR1, &act, NULL);
}

static void
make_daemon() {
  pid_t pid, sid;

  pid = fork();
  if (pid < 0) {
    printf("fork process error\n");
    exit(-1);
  }   
  if (pid > 0) {
    printf("\n"
      "*****************************************************\n"
      "* qserver will be running as a daemon. PID %-8d *\n"
      "*****************************************************\n\n",
      pid);
    signal(SIGCHLD, SIG_IGN);
    exit(0);
  }   

  umask(0);
  sid = setsid();
  if (sid < 0) {
    exit(-1);
  }   

  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
}

static void
save_pid() {
  FILE *file;
  char pid[10];
  int  n;

  file = fopen("qserver.pid", "w");
  if (file == NULL) {
    qstdout("open pid file error\n");
    exit(-1);
  }
  n = snprintf(pid, sizeof(pid), "%d", getpid());
  fwrite(pid, sizeof(char), n, file);
  fclose(file);
}

static int
set_core_size() {
  struct rlimit rlim_core;

  rlim_core.rlim_cur = RLIM_INFINITY; /* Soft limit */
  rlim_core.rlim_max = RLIM_INFINITY; /* Hard limit (ceiling
                                         for rlim_cur) */
  return setrlimit(RLIMIT_CORE, &rlim_core);
}

static int
server_init() {
  qassert(config.worker > 0);
  qassert(server == NULL);

  server = qcalloc(sizeof(qserver_t));
  if (server == NULL) {
    goto error;
  }

  if (config.daemon) {
    make_daemon();
  }
  save_pid();
  if (set_core_size() < 0) {
    qstdout("set core size error\n");
    goto error;
  }

  if (qlogger_new(config.worker + 1) < 0) {
    goto error;
  }

  engine = qengine_new();
  if (engine == NULL) {
    goto error;
  }
  if (init_server_event(server) < 0) {
    goto error;
  }
  qmailbox_init(&box, server_msg_handler,
                engine, server);
  server->descriptors = qcalloc(QID_MAX * sizeof(qdescriptor_t*));
  if (server->descriptors == NULL) {
    goto error;
  }

  if (chdir(config.script_path) != 0) {
    qstdout("chdir %s error: %s\n", config.script_path, strerror(errno));
    goto error;
  }
  if (init_workers(server) < 0) {
    goto error;
  }
  setup_signal();

  server_start(server);

  return 0;

error:
  return -1;
}

static void
destroy_threads() {
  int         i;

  for (i = 1; i <= config.worker; ++i) {
    qworker_destroy(workers[i]);
  }
  qlogger_destroy();
}

static void
destroy_server() {
  destroy_threads();
  qconfig_free();
}

int
qserver_run() {
  if (server_init() != 0) {
    return -1;
  }
  qengine_loop(engine);
  destroy_server();
  return 0;
}

void
qserver_quit() {
  engine->quit = 1;
}
