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
#include "qlog.h"
#include "qlog_thread.h"
#include "qmailbox.h"
#include "qmsg.h"
#include "qserver.h"
#include "qsignal.h"
#include "qthread_log.h"

pthread_key_t g_thread_log_key = PTHREAD_ONCE_INIT;
qlog_thread_t *g_log_thread = NULL;

static int
logger_msg_handler(qmsg_t *msg, void *reader) {
  qlog_t        *log;
  qlog_thread_t *thread;
  
  thread = (qlog_thread_t*)reader;
  log = msg->args.log.log;
  printf("%s\n", log->buff);
  qfree(log);

  return 1;
}

static void*
log_thread_main_loop(void *arg) {
  qlog_thread_t *thread;

  thread = (qlog_thread_t*)arg;
  qserver_worker_started();
  qengine_loop(thread->engine);

#if 0
  /* now the server terminate, do the clean work */
  for (i = 0; i < g_log_thread->thread_num; ++i) {
    signal = g_log_thread->signals[i];
    fd = qsignal_get_fd(signal);
    qengine_del_event(g_log_thread->engine, fd, QEVENT_READ);
    log_thread_box(0, -1, signal);
  }
#endif
  qengine_destroy(g_log_thread->engine);

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

  engine = g_log_thread->engine;
  t = engine->timer_mng.now;
  localtime_r(&t, &tm);
  strftime(g_log_thread->time_buff, sizeof(g_log_thread->time_buff),
           "[%m-%d %T", &tm);
}

int
qlog_thread_new(int thread_num) {
  g_log_thread = qcalloc(sizeof(qlog_thread_t));
  if (g_log_thread == NULL) {
    return -1;
  }

  if (pthread_key_create(&g_thread_log_key, log_key_destroy) < 0) {
    return -1;
  }

  g_log_thread->engine = qengine_new();
  if (g_log_thread->engine == NULL) {
    return -1;
  }
  qmailbox_init(&(g_log_thread->box), logger_msg_handler,
                g_log_thread->engine,  g_log_thread);

  g_log_thread->thread_num = thread_num;
  log_time_handler(NULL);
  qengine_add_timer(g_log_thread->engine, 1000, log_time_handler,
                    1000, NULL);
  pthread_create(&g_log_thread->id, NULL,
                 log_thread_main_loop, g_log_thread);

  return 0;
}

void
qlog_thread_destroy() {
  /* wait for the thread */
  pthread_join(g_log_thread->id, NULL);
}

void
qlog_thread_add(qlog_t *log) {
  qmsg_t *msg;

  msg = qmsg_new(log->idx, 0);
  if (msg == NULL) {
    qfree(log);
    return;
  }
  qmsg_init_log(msg, log);
  qmailbox_add(&(g_log_thread->box), msg);
}
