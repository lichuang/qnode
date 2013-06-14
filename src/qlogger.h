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
  int           fd;
  int           log_size;
} qlogger_t;

int  qlogger_new(int thread_num);
void qlogger_destroy();
void qlogger_add(qlog_t *log);
void qlogger_send(qmsg_t *msg);
void qlogger_open_file();

extern qlogger_t     *logger;
extern pthread_key_t  thread_log_key;

#endif  /* __qlogger_H__ */
