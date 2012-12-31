/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qassert.h"
#include "qconfig.h"
#include "qengine.h"
#include "qlist.h"
#include "qlog.h"
#include "qlog_thread.h"
#include "qmailbox.h"
#include "qmalloc.h"
#include "qmsg.h"
#include "qserver.h"
#include "qthread_log.h"

qlog_thread_t *g_log_thread = NULL;

static void thread_log_box(int fd, int flags, void *data) {
  qlog_thread_t *thread = (qlog_thread_t*)data;
  int i = 0;
  qlist_t *list;
  qlog_t *log;
  for (i = 0; i < g_server->config->thread_num; ++i) {
    qthread_log_t *thread_log = g_server->thread_log[i];
    qthread_log_fetch(thread_log, &(g_log_thread->lists[i]));
  }
  qsignal_recv(thread->signal);
  qsignal_active(thread->signal, 0);

  qlist_t *pos, *next;
  for (i = 0; i < g_server->config->thread_num; ++i) {
    list = g_log_thread->lists[i];
    for (pos = list->next; pos != list; ) {
      log = qlist_entry(pos, qlog_t, entry);
      next = pos->next;
      qlist_del_init(&(log->entry));

      log->n += sprintf(log->buff + log->n, " %s:%d ", log->file, log->line);
      vsprintf(log->buff + log->n, log->format, log->args);
      printf("%s\n", log->buff);

      qfree(log);
      pos = next;
    }
  }
}

static void* main_loop(void *arg) {
  qlog_thread_t *thread = (qlog_thread_t*)arg;
  qengine_loop(thread->engine);
  return NULL;
}

int qlog_thread_new(int thread_num) {
  g_log_thread = qalloc_type(qlog_thread_t);
  if (g_log_thread == NULL) {
    return -1;
  }
  g_log_thread->engine = qengine_new();
  if (g_log_thread->engine == NULL) {
    qfree(g_log_thread);
    return -1;
  }
  g_log_thread->lists = (qlist_t**)qmalloc(thread_num * sizeof(qlist_t*));
  g_log_thread->signal = qsignal_new();
  int fd = qsignal_get_fd(g_log_thread->signal);
  qengine_add_event(g_log_thread->engine, fd, QEVENT_READ, thread_log_box, g_log_thread);
  int result;
  result = pthread_create(&g_log_thread->id, NULL, main_loop, g_log_thread);
  qassert(result == 0);
  return 0;
}

void qlog_thread_active() {
  if (qsignal_active(g_log_thread->signal, 1) == 0) {
    qsignal_send(g_log_thread->signal);
  }
}
