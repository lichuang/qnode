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
#include "qcore.h"
#include "qengine.h"
#include "qdefines.h"
#include "qfreelist.h"
#include "qlog.h"
#include "qlogger.h"
#include "qmailbox.h"
#include "qmsg.h"
#include "qmmsg.h"
#include "qnet.h"
#include "qserver.h"
#include "qsignal.h"
#include "qsocket.h"
#include "qthread_log.h"
#include "qwmsg.h"
#include "qworker.h"

int test_flag = 0;

extern qmsg_pt* server_msg_handlers[];

static qengine_t *engine;
static qmailbox_t box;
static qevent_t   event;

static void server_accept(int fd, int flags, void *data);
static int  init_server_event();
static int  server_msg_handler(qmsg_t *msg, void *reader);
static void server_start();
static int  init_workers();
static void signal_handler(int sig);
static void setup_signal();
static int  init_server();
static void destroy_threads();
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
init_server_event() {
  return 0;
  int error;
  int fd = qnet_tcp_listen(22222, "127.0.0.1", &error);
  if (fd < 0) {
    return -1;
  }

  qevent_init(&event, fd, server_accept, NULL, NULL);
  if (qevent_add(engine, &event, QEVENT_READ) < 0) {
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
server_start() {
  qid_t  tid;
  qmsg_t *msg;

  tid = qserver_worker();
  msg = qwmsg_start_new(QMAINTHREAD_TID, tid);
  if (msg == NULL) {
    return;
  }
  qworker_send(msg);
}

static int
init_workers() {
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
    
  return QOK;

error:
  /*
   * no need to free memory, cause this function called
   * when server start, just end the server wiil be fine
   */
  return QERROR;
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

  /* Soft limit */
  rlim_core.rlim_cur = RLIM_INFINITY;
  /* Hard limit (ceiling for rlim_cur) */
  rlim_core.rlim_max = RLIM_INFINITY;

  return setrlimit(RLIMIT_CORE, &rlim_core);
}

static int
init_server() {
  qassert(config.worker > 0);

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

  if (qbuffer_init_freelist() < 0) {
    goto error;
  }

  engine = qengine_new();
  if (engine == NULL) {
    goto error;
  }
  if (init_server_event() < 0) {
    goto error;
  }
  qmailbox_init(&box, server_msg_handler,
                engine, NULL);
  qsocket_init_free_list();
  if (chdir(config.script_path) != 0) {
    qstdout("chdir %s error: %s\n", config.script_path, strerror(errno));
    goto error;
  }
  if (init_workers() < 0) {
    goto error;
  }
  setup_signal();

  server_start();

  return QOK;

error:
  return QERROR;
}

static void
destroy_threads() {
  int i;

  for (i = 1; i <= config.worker; ++i) {
    qworker_destroy(workers[i]);
  }
  qlogger_destroy();
}

int
qserver_run() {
  if (init_server() != QOK) {
    return QERROR;
  }
  qengine_loop(engine);
  destroy_threads();
  qconfig_free();
  qsocket_destroy_free_list();
  qbuffer_destroy_freelist();
  return QOK;
}

void
qserver_quit() {
  engine->quit = 1;
}
