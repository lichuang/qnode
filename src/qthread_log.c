/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "qalloc.h"
#include "qassert.h"
#include "qatomic.h"
#include "qengine.h"
#include "qdefines.h"
#include "qlog.h"
#include "qthread_log.h"

extern pthread_key_t g_thread_log_key;

qthread_log_t*
qthread_log_init(int idx) {
  qthread_log_t *thread_log;

  thread_log = qcalloc(sizeof(qthread_log_t));
  if (thread_log == NULL) {
    return NULL;
  }
  qlist_entry_init(&thread_log->lists[0]);
  qlist_entry_init(&thread_log->lists[1]);
  thread_log->write = &thread_log->lists[0];
  thread_log->read  = &thread_log->lists[0];
  if (pthread_setspecific(g_thread_log_key, thread_log) < 0) {
    qfree(thread_log);
    return NULL;
  }
  qassert(pthread_getspecific(g_thread_log_key) != NULL);
  thread_log->idx = idx;

  return thread_log;
}

qlog_t*
qthread_log_get() {
  qlog_t        *log;
  qthread_log_t *thread_log;

  thread_log = (qthread_log_t*)pthread_getspecific(g_thread_log_key);
  if (thread_log == NULL) {
    return NULL;
  }
  
  log = qcalloc(sizeof(qlog_t));
  if (log == NULL) {
    return NULL;
  }
  log->idx = thread_log->idx;
  /*
  log->n = strlen(thread_log->engine->time_buff);
  strcpy(log->buff, thread_log->engine->time_buff);
  */
  qlist_add_tail(&log->entry, thread_log->write);

  return log;
}

void
qthread_log_fetch(qthread_log_t *log, qlist_t **list) {
  qlist_t *read;

  *list = NULL;
  /* first save the read ptr */
  read = log->read;
  /* second change the read ptr to the write ptr */
  qatomic_ptr_xchg(&(log->read), log->write);
  /* last change the write ptr to the read ptr saved before and return to list */
  *list = qatomic_ptr_xchg(&(log->write), read);
}
