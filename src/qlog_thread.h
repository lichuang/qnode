/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QLOG_THREAD_H__
#define __QLOG_THREAD_H__

#include <pthread.h>

struct qengine_t;
struct qsignal_t;

typedef struct qlog_thread_t {
  unsigned short started;
  pthread_t id;
  struct qengine_t *engine;
  struct qsignal_t **signals;
} qlog_thread_t;

int qlog_thread_new(int thread_num);
void qlog_thread_active(int idx);

extern qlog_thread_t *g_log_thread;
extern pthread_key_t g_thread_log_key;

#endif  /* __QLOG_THREAD_H__ */
