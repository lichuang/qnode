/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QLOG_THREAD_H__
#define __QLOG_THREAD_H__

#include <pthread.h>
#include "qcore.h"

struct qengine_t;
struct qsignal_t;

typedef struct qlog_thread_t {
  unsigned          started:1;
  unsigned          stop:1;
  pthread_t         id;
  int               thread_num;
  struct qengine_t  *engine;
  struct qsignal_t  **signals;
} qlog_thread_t;

int  qlog_thread_new(qmem_pool_t *pool, int thread_num);
void qlog_thread_destroy();
void qlog_thread_active(int idx);

extern qlog_thread_t *g_log_thread;
extern pthread_key_t  g_thread_log_key;

#endif  /* __QLOG_THREAD_H__ */
