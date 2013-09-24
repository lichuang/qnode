/*
 * See Copyright Notice in qnode.h
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "qalloc.h"
#include "qassert.h"
#include "qconfig.h"
#include "qdefines.h"
#include "qengine.h"
#include "qlist.h"
#include "qlmsg.h"
#include "qlog.h"
#include "qlogger.h"
#include "qmsg.h"
#include "qserver.h"
#include "qsignal.h"
#include "qthread_log.h"

extern qmsg_pt* logger_msg_handlers[];

pthread_key_t thread_log_key = PTHREAD_ONCE_INIT;
qlogger_t    *logger         = NULL;

static int    logger_msg_handler(qmsg_t *msg, void *reader);
static void*  logger_main(void *arg);
static void   log_key_destroy(void *value);
static void   log_time_handler(void *data);
static void   logger_handle_msglist_done(void *reader);
static void   destroy_logs();

static int
logger_msg_handler(qmsg_t *msg, void *reader) {
  return (*logger_msg_handlers[msg->type])(msg, reader);
}

void
qlogger_open_file() {
  qstring_t   file;
  char        buff[30] = {'\0'};
  time_t      t;  
  struct tm   tm; 

  t = time(NULL);
  memset(&tm, 0, sizeof(struct tm));
  localtime_r(&t, &tm);
  strftime((char*)(&buff[0]), sizeof(buff), "%Y%m%d-%H%M%S", &tm);

  logger->log_size = 0;
  if (logger->fd != -1) {
    fsync(logger->fd);
    close(logger->fd);
  }
  file = qstring_new(config.log_path);
  file = qstring_catvprintf(file, "/qserver_%s.log", buff);

  logger->fd = open(file, O_CREAT | O_TRUNC | O_RDWR,
                    S_IWUSR | S_IRUSR | S_IWOTH | S_IROTH | S_IRGRP | S_IWGRP);
  if (logger->fd == -1) {
    qstdout("open log file %s error: %s\n", file, strerror(errno));
    exit(-1);
  }
  qstring_destroy(file);
}

static void*
logger_main(void *arg) {
  qlogger_t *thread;

  thread = (qlogger_t*)arg;
  thread->running = 1;
  qengine_loop(thread->engine);

  qmailbox_free(&logger->box);
  qengine_destroy(logger->engine);
  destroy_logs();
  qlog_destroy_free_list();

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

  UNUSED(data);

  t = time(NULL);
  localtime_r(&t, &tm);
  strftime(logger->time_buff, sizeof(logger->time_buff),
           "%Y%m%d %H:%M:%S", &tm);
}

static void
destroy_logs() {
  if (!qlist_empty(&(logger->free_list))) {
    qlog_free(&(logger->free_list));
    qlist_entry_init(&(logger->free_list));
  }
  qassert(qlist_empty(&logger->free_list));
}

static void
logger_handle_msglist_done(void *reader) {
  UNUSED(reader);
  destroy_logs();
}

int
qlogger_new(int thread_num) {
  qlog_init_free_list();
  logger = qcalloc(sizeof(qlogger_t));
  if (logger == NULL) {
    return QERROR;
  }

  if (pthread_key_create(&thread_log_key, log_key_destroy) < 0) {
    return QERROR;
  }

  logger->fd = -1;
  logger->log_size = 0;
  if (config.daemon) {
    qlogger_open_file();
  }
  logger->engine = qengine_new();
  if (logger->engine == NULL) {
    return QERROR;
  }
  qmailbox_init(&(logger->box), logger_msg_handler,
                logger->engine, logger);
  logger->box.done = logger_handle_msglist_done;

  logger->thread_num = thread_num;
  qlist_entry_init(&(logger->free_list));
  log_time_handler(NULL);
  qtimer_add(logger->engine, 1000, log_time_handler,
             NULL, 1000, NULL);
  logger->running = 0;
  pthread_create(&logger->id, NULL,
                 logger_main, logger);
  while (!logger->running) {
    usleep(100);
  }

  return QOK;
}

void
qlogger_destroy() {
  /* wait for the thread */
  pthread_join(logger->id, NULL);
  qfree(logger);
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
