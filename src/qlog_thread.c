/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qassert.h"
#include "qengine.h"
#include "qlist.h"
#include "qlog.h"
#include "qlog_thread.h"
#include "qmailbox.h"
#include "qmalloc.h"
#include "qmsg.h"

qlog_thread_t *g_log_thread = NULL;

static void thread_log_box(int fd, int flags, void *data) {
}

static void* main_loop(void *arg) {
  qlog_thread_t *thread = (qlog_thread_t*)arg;
  qengine_loop(thread->engine);
  return NULL;
}

int qlog_thread_new(int box_num) {
  g_log_thread = qalloc_type(qlog_thread_t);
  if (g_log_thread == NULL) {
    return -1;
  }
  g_log_thread->engine = qengine_new();
  if (g_log_thread->engine == NULL) {
    qfree(g_log_thread);
    return -1;
  }
  g_log_thread->log_box = (qmailbox_t**)qmalloc(box_num * sizeof(qmailbox_t*));
  if (g_log_thread->log_box == NULL) {
    return -1;
  }
  int i;
  for (i = 0; i < box_num; ++i) {
    qmailbox_t *box = qmailbox_new(thread_log_box, g_log_thread);
    if (box == NULL) {
    }
    g_log_thread->log_box[i] = box;
    qmailbox_active(g_log_thread->engine, box);
  }
  int result;
  result = pthread_create(&g_log_thread->id, NULL, main_loop, g_log_thread);
  qassert(result == 0);
  return 0;
}
