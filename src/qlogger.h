/*
 * See Copyright Notice in qnode.h
 */

#ifndef __qlogger_H__
#define __qlogger_H__

#include <pthread.h>
#include "qcore.h"
#include "qlist.h"
#include "qmailbox.h"

typedef struct qlogger_t {
  qmailbox_t       box;
  pthread_t        id;
  int              thread_num;
  qengine_t       *engine;
  char             time_buff[20];
  int              fd;
  int              log_size;
  qlist_t          free_list;
  volatile int     running:1;
  qthread_start_pt done;
} qlogger_t;

int  qlogger_new(int thread_num, qthread_start_pt done);
void qlogger_destroy();
void qlogger_add(qlog_t *log);
void qlogger_send(qmsg_t *msg);
void qlogger_open_file();

extern qlogger_t     *logger;
extern pthread_key_t  thread_log_key;

#endif  /* __qlogger_H__ */
