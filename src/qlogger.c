/*
 * See Copyright Notice in qnode.h
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "qalloc.h"
#include "qassert.h"
#include "qconfig.h"
#include "qdefines.h"
#include "qengine.h"
#include "qlist.h"
#include "qlmsg.h"
#include "qlog.h"
#include "qlogger.h"
#include "qmailbox.h"
#include "qmsg.h"
#include "qserver.h"
#include "qsignal.h"
#include "qthread_log.h"

extern qmsg_func_t* logger_msg_handlers[];

pthread_key_t thread_log_key = PTHREAD_ONCE_INIT;
qlogger_t    *logger         = NULL;

static int    logger_msg_handler(qmsg_t *msg, void *reader);
static void*  log_thread_main_loop(void *arg);
static void   log_key_destroy(void *value);
static void   log_time_handler(void *data);

static int
logger_msg_handler(qmsg_t *msg, void *reader) {
  return (*logger_msg_handlers[msg->type])(msg, reader);
}

void
qlogger_open_file() {
  qstring_t   file;
  char        buff[30];
  time_t      t;  
  struct tm   tm; 

  t = time(NULL);
  localtime_r(&t, &tm);
  strftime((char*)(&buff[0]), sizeof(buff), "%m-%d-%T", &tm);

  /*
  if (log_size < kLogFileSize) {
    return;
  }
  */
  logger->log_size = 0;
  if (logger->fd != -1) {
    fsync(logger->fd);
    close(logger->fd);
  }
  file = qstring_new(server->config->log_path);
  file = qstring_catvprintf(file, "/qserver_%s.log", buff);

  logger->fd = open(file, O_CREAT | O_TRUNC | O_RDWR,
                    S_IWUSR | S_IRUSR | S_IWOTH | S_IROTH | S_IRGRP | S_IWGRP);
  qstring_destroy(file);
}

static void*
log_thread_main_loop(void *arg) {
  qlogger_t *thread;

  thread = (qlogger_t*)arg;
  qserver_worker_started();
  qengine_loop(thread->engine);

#if 0
  /* now the server terminate, do the clean work */
  for (i = 0; i < logger->thread_num; ++i) {
    signal = logger->signals[i];
    fd = qsignal_get_fd(signal);
    qengine_del_event(logger->engine, fd, QEVENT_READ);
    log_thread_box(0, -1, signal);
  }
#endif
  qengine_destroy(logger->engine);

  return NULL;
}

static void
log_key_destroy(void *value) {
  UNUSED(value);
}

static void
log_time_handler(void *data) {
  time_t     t;
  struct tm  tm;
  qengine_t *engine;

  UNUSED(data);

  engine = logger->engine;
  t = engine->timer_mng.now;
  localtime_r(&t, &tm);
  strftime(logger->time_buff, sizeof(logger->time_buff),
           "[%m-%d %T", &tm);
}

int
qlogger_new(int thread_num) {
  logger = qcalloc(sizeof(qlogger_t));
  if (logger == NULL) {
    return -1;
  }

  if (pthread_key_create(&thread_log_key, log_key_destroy) < 0) {
    return -1;
  }

  logger->fd = 0;
  logger->log_size = 0;
  qlogger_open_file();
  logger->engine = qengine_new();
  if (logger->engine == NULL) {
    return -1;
  }
  qmailbox_init(&(logger->box), logger_msg_handler,
                logger->engine,  logger);

  logger->thread_num = thread_num;
  log_time_handler(NULL);
  qengine_add_timer(logger->engine, 1000, log_time_handler,
                    1000, NULL);
  pthread_create(&logger->id, NULL,
                 log_thread_main_loop, logger);

  return 0;
}

void
qlogger_destroy() {
  /* wait for the thread */
  pthread_join(logger->id, NULL);
}

void
qlogger_add(qlog_t *log) {
  qmsg_t *msg;

  msg = qlmsg_log_new(log, log->idx);
  if (msg == NULL) {
    qfree(log);
    return;
  }
  qmailbox_add(&(logger->box), msg);
}

void qlogger_send(qmsg_t *msg) {
  qmailbox_add(&(logger->box), msg);
}
