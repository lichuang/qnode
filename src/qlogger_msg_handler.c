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

#define NONE         "\033[m"
#define RED          "\033[0;32;31m"

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
  log = lmsg->log;

  if (config.daemon) {
    logger->log_size += log->size;
    if (logger->log_size > config.log_size) {
      qlogger_open_file();
    }
    write(logger->fd, log->buff, log->size);
  } else {
    if (log->level > QLOG_ERR) {
      printf("%s", log->buff);
    } else {
      printf(RED"%s"NONE, log->buff);
    }
  }

  /* seems freelist no effect, so free directly */
  //qlist_add_tail(&(log->fentry), &(logger->free_list));
  qfree(log);

  return QOK;
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

  return QOK;
}
