/*
 * See Copyright Notice in qnode.h
 */

#include "qalloc.h"
#include "qlmsg.h"
#include "qlog.h"
#include "qlogger.h"

static int logger_handle_log_msg(qmsg_t *msg, void *reader);

qmsg_func_t* g_logger_msg_handlers[] = {
  &logger_handle_log_msg,     /* log */
};

static int
logger_handle_log_msg(qmsg_t *msg, void *reader) {
  qlmsg_log_t   *lmsg;
  qlog_t        *log;
  qlogger_t *thread;
  
  thread = (qlogger_t*)reader;
  lmsg = (qlmsg_log_t*)msg;
  log = lmsg->log;
  printf("%s\n", log->buff);
  qfree(log);

  return 0;
}
