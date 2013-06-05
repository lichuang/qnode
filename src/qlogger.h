/*
 * See Copyright Notice in qnode.h
 */

#ifndef __qlogger_H__
#define __qlogger_H__

#include <pthread.h>
#include "qcore.h"
#include "qmailbox.h"

typedef struct qlogger_t {
  qmailbox_t    box;
  pthread_t     id;
  int           thread_num;
  qengine_t    *engine;
  char          time_buff[20];
} qlogger_t;

int  qlogger_new(int thread_num);
void qlogger_destroy();
void qlogger_add(qlog_t *log);

extern qlogger_t     *g_logger;
extern pthread_key_t  g_thread_log_key;

#endif  /* __qlogger_H__ */
