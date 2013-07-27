/*
 * See Copyright Notice in qnode.h
 */

#include <unistd.h>
#include <signal.h>
#include "qalloc.h"
#include "qconfig.h"
#include "qlmsg.h"
#include "qlog.h"
#include "qlogger.h"
#include "qserver.h"

static int  logger_log_handler(qmsg_t *msg, void *reader);
static int  logger_signal_handler(qmsg_t *msg, void *reader);
static void destroy_log_msg(qmsg_t *msg);

qmsg_pt* logger_msg_handlers[] = {
  &logger_log_handler,
  &logger_signal_handler,
};

static int
logger_log_handler(qmsg_t *msg, void *reader) {
  qlmsg_log_t   *lmsg;
  qlog_t        *log;
  qlogger_t     *logger;
  
  logger = (qlogger_t*)reader;
  lmsg = (qlmsg_log_t*)msg;
  lmsg->destroy = destroy_log_msg;
  log = lmsg->log;

  logger->log_size += log->size;
  if (logger->log_size > config.log_size) {
    qlogger_open_file();
  }
  if (config.daemon) {
    write(logger->fd, log->buff, log->size);
  } else {
    printf("%s", log->buff);
  }

  return 0;
}

static int
logger_signal_handler(qmsg_t *msg, void *reader) {
  qlmsg_signal_t *signal;
  qlogger_t      *logger;

  signal = (qlmsg_signal_t*)msg;
  logger = (qlogger_t*)reader;
  
  switch (signal->signo) {
    case SIGTERM:
    case SIGQUIT:
    case SIGINT:
      logger->engine->quit = 1;
      break;
    default:
      break;
  }
  return 0;
}

static void
destroy_log_msg(qmsg_t *msg) {
  qlmsg_log_t   *lmsg;
  qlog_t        *log;
  
  lmsg = (qlmsg_log_t*)msg;
  log = lmsg->log;

  qlist_add_tail(&(log->fentry), &(logger->free_list));
}
