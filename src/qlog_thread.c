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

static void thread_log_box(int fd, int flags, void *data) {
  int           i, idx;
  qsignal_t     *signal;
  qlist_t       *list, *pos, *next;
  qlog_t        *log;
  qthread_log_t *thread_log;

  UNUSED(fd);
  UNUSED(flags);

  signal = (qsignal_t*)data;
  for (i = 0; ; i++) {
    if (signal == g_log_thread->signals[i]) {
      idx = i;
      break;
    }
  }
  thread_log = g_server->thread_log[idx];
  qthread_log_fetch(thread_log, &list);
  /*
   * -1 means destroy the log thread
   */
  if (flags != -1) {
    qsignal_recv(signal);
    qsignal_active(signal, 0);
  }

  for (pos = list->next; pos != list; ) {
    log = qlist_entry(pos, qlog_t, entry);
    next = pos->next;
    qlist_del_init(&(log->entry));

    log->n = sprintf(log->buff, "%s %d", g_log_thread->time_buff, log->idx);
    log->n += sprintf(log->buff + log->n, " %s:%d ", log->file, log->line);
    vsprintf(log->buff + log->n, log->format, log->args);
    printf("%s\n", log->buff);
    if (flags == -1) {
      /* do flush I/O work */
    }

    qfree(log);
    pos = next;
  }
}

static void* log_thread_main_loop(void *arg) {
  int         i, fd;
  qsignal_t  *signal;
  qlog_thread_t *thread;

  thread = (qlog_thread_t*)arg;
  thread->started = 1;
  qserver_worker_started();
  while (!thread->stop && qengine_loop(thread->engine) == 0) {
  }

  /*
   * now the server terminate, do the clean work
   */
  for (i = 0; i < g_log_thread->thread_num; ++i) {
    signal = g_log_thread->signals[i];
    fd = qsignal_get_fd(signal);
    qengine_del_event(g_log_thread->engine, fd, QEVENT_READ);
    thread_log_box(0, -1, signal);
  }
  qengine_destroy(g_log_thread->engine);

  return NULL;
}

static void log_key_destroy(void *value) {
  UNUSED(value);
}

static void log_time_handler(void *data) {
  time_t     t;
  struct tm  tm;
  qengine_t *engine;

  UNUSED(data);

  engine = g_log_thread->engine;
  t = engine->timer_mng.now;
  localtime_r(&t, &tm);
  strftime(g_log_thread->time_buff, sizeof(g_log_thread->time_buff),
           "[%m-%d %T]", &tm);
}

int qlog_thread_new(int thread_num) {
  int i;
  int fd;

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

  g_log_thread->thread_num = thread_num;
  g_log_thread->signals = qcalloc(thread_num * sizeof(qsignal_t*));
  if (g_log_thread->signals == NULL) {
    return -1;
  }

  for (i = 0; i < thread_num; ++i) {
    g_log_thread->signals[i] = qsignal_new();
    fd = qsignal_get_fd(g_log_thread->signals[i]);
    qengine_add_event(g_log_thread->engine, fd, QEVENT_READ,
                      thread_log_box, g_log_thread->signals[i]);
  }
  g_log_thread->started = 0;
  log_time_handler(NULL);
  qengine_add_timer(g_log_thread->engine, 1000, log_time_handler,
                    1000, NULL);
  pthread_create(&g_log_thread->id, NULL,
                 log_thread_main_loop, g_log_thread);

  return 0;
}

void qlog_thread_destroy() {
  /* set the stop flag and wake up the log thread */
  g_log_thread->stop = 1;
  qlog_thread_active(0);
  /* wait for the thread */
  pthread_join(g_log_thread->id, NULL);
}

void qlog_thread_active(int idx) {
  if (qsignal_active(g_log_thread->signals[idx], 1) == 0) {
    qsignal_send(g_log_thread->signals[idx]);
  }
}
