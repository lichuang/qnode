/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QLOG_THREAD_H__
#define __QLOG_THREAD_H__

#include <pthread.h>
#include "qcore.h"
#include "qmailbox.h"

typedef struct qlog_thread_t {
  qmailbox_t    box;
  pthread_t     id;
  int           thread_num;
  qengine_t    *engine;
  char          time_buff[20];
} qlog_thread_t;

int  qlog_thread_new(int thread_num);
void qlog_thread_destroy();
void qlog_thread_add(qlog_t *log);

extern qlog_thread_t *g_log_thread;
extern pthread_key_t  g_thread_log_key;

#endif  /* __QLOG_THREAD_H__ */
