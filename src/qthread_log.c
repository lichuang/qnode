/*
 * See Copyright Notice in qnode.h
 */

#include <pthread.h>
#include "qlog.h"
#include "qmalloc.h"
#include "qthread_log.h"

static pthread_key_t qthread_log_key;

static qthread_log_t* thread_log_new() {
  qthread_log_t *thread_log = qalloc_type(qthread_log_t);
  if (thread_log == NULL) {
    return NULL;
  }
  qlist_entry_init(&thread_log->lists[0]);
  qlist_entry_init(&thread_log->lists[1]);
  thread_log->write = &thread_log->lists[0];
  thread_log->read  = &thread_log->lists[0];
  return thread_log;
}

struct qlog_t* qthread_log_get() {
  qthread_log_t *thread_log = NULL;
  thread_log = pthread_getspecific(qthread_log_key);
  if (thread_log == NULL) {
    thread_log = thread_log_new();
    if (thread_log == NULL) {
      return NULL;
    }
     if (pthread_setspecific(qthread_log_key, thread_log) < 0) {
       qfree(thread_log);
       return NULL;
     }
  }
  
  qlog_t *log = qalloc_type(qlog_t);
  if (log == NULL) {
    return NULL;
  }
  qlist_add_tail(&log->list, thread_log->write);
  return log;
}
