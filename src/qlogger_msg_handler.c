/*
 * See Copyright Notice in qnode.h
 */

#include <signal.h>
#include "qalloc.h"
#include "qlmsg.h"
#include "qlog.h"
#include "qlogger.h"

static int logger_log_handler(qmsg_t *msg, void *reader);
static int logger_signal_handler(qmsg_t *msg, void *reader);

qmsg_func_t* logger_msg_handlers[] = {
  &logger_log_handler,
  &logger_signal_handler,
};

static int
logger_log_handler(qmsg_t *msg, void *reader) {
  qlmsg_log_t   *lmsg;
  qlog_t        *log;
  qlogger_t     *thread;
  
  thread = (qlogger_t*)reader;
  lmsg = (qlmsg_log_t*)msg;
  log = lmsg->log;
  printf("%s\n", log->buff);
  qfree(log);

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
