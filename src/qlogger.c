/*
 * See Copyright Notice in qnode.h
 */

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

extern qmsg_func_t* g_logger_msg_handlers[];

pthread_key_t g_thread_log_key = PTHREAD_ONCE_INIT;
qlogger_t *g_logger = NULL;

static int
logger_msg_handler(qmsg_t *msg, void *reader) {
  return (*g_logger_msg_handlers[msg->type])(msg, reader);
}

static void*
log_thread_main_loop(void *arg) {
  qlogger_t *thread;

  thread = (qlogger_t*)arg;
  qserver_worker_started();
  qengine_loop(thread->engine);

#if 0
  /* now the server terminate, do the clean work */
  for (i = 0; i < g_logger->thread_num; ++i) {
    signal = g_logger->signals[i];
    fd = qsignal_get_fd(signal);
    qengine_del_event(g_logger->engine, fd, QEVENT_READ);
    log_thread_box(0, -1, signal);
  }
#endif
  qengine_destroy(g_logger->engine);

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

  engine = g_logger->engine;
  t = engine->timer_mng.now;
  localtime_r(&t, &tm);
  strftime(g_logger->time_buff, sizeof(g_logger->time_buff),
           "[%m-%d %T", &tm);
}

int
qlogger_new(int thread_num) {
  g_logger = qcalloc(sizeof(qlogger_t));
  if (g_logger == NULL) {
    return -1;
  }

  if (pthread_key_create(&g_thread_log_key, log_key_destroy) < 0) {
    return -1;
  }

  g_logger->engine = qengine_new();
  if (g_logger->engine == NULL) {
    return -1;
  }
  qmailbox_init(&(g_logger->box), logger_msg_handler,
                g_logger->engine,  g_logger);

  g_logger->thread_num = thread_num;
  log_time_handler(NULL);
  qengine_add_timer(g_logger->engine, 1000, log_time_handler,
                    1000, NULL);
  pthread_create(&g_logger->id, NULL,
                 log_thread_main_loop, g_logger);

  return 0;
}

void
qlogger_destroy() {
  /* wait for the thread */
  pthread_join(g_logger->id, NULL);
}

void
qlogger_add(qlog_t *log) {
  qmsg_t *msg;

  msg = qlmsg_log_new(log, log->idx);
  if (msg == NULL) {
    qfree(log);
    return;
  }
  qmailbox_add(&(g_logger->box), msg);
}
