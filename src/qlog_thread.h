/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QLOG_THREAD_H__
#define __QLOG_THREAD_H__

#include <pthread.h>

struct qengine;
struct qmailbox_t;

typedef struct qlog_thread_t {
  pthread_t id;
  struct qengine_t *engine;
  struct qmailbox **log_box;
} qlog_thread_t;

int qlog_thread_new(int box_num);

extern qlog_thread_t *g_log_thread;

#endif  /* __QLOG_THREAD_H__ */
