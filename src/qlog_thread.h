/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QLOG_THREAD_H__
#define __QLOG_THREAD_H__

#include <pthread.h>

struct qengine_t;
struct qsignal_t;

typedef struct qlog_thread_t {
  pthread_t id;
  struct qengine_t *engine;
  struct qsignal_t *signal;
  struct qlist_t **lists;
} qlog_thread_t;

int qlog_thread_new(int thread_num);
void qlog_thread_active();

extern qlog_thread_t *g_log_thread;

#endif  /* __QLOG_THREAD_H__ */
