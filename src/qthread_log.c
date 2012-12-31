/*
 * See Copyright Notice in qnode.h
 */

#include <pthread.h>
#include <string.h>
#include "qassert.h"
#include "qengine.h"
#include "qlog.h"
#include "qmalloc.h"
#include "qthread_log.h"

static pthread_key_t qthread_log_key;

void qthread_log_destroy(void *value) {
}

int qthread_log_init(struct qengine_t* engine) {
  if (pthread_key_create(&qthread_log_key, qthread_log_destroy) < 0) {
    return -1;
  }
  qthread_log_t *thread_log = qalloc_type(qthread_log_t);
  if (thread_log == NULL) {
    return -1;
  }
  qlist_entry_init(&thread_log->lists[0]);
  qlist_entry_init(&thread_log->lists[1]);
  thread_log->write = &thread_log->lists[0];
  thread_log->read  = &thread_log->lists[0];
  thread_log->engine = engine;
  if (pthread_setspecific(qthread_log_key, thread_log) < 0) {
    qfree(thread_log);
    return -1;
  }
  qassert(pthread_getspecific(qthread_log_key) != NULL);
  return 0;
}

struct qlog_t* qthread_log_get() {
  qthread_log_t *thread_log = NULL;
  thread_log = (qthread_log_t*)pthread_getspecific(qthread_log_key);
  if (thread_log == NULL) {
    return NULL;
  }
  
  qlog_t *log = qalloc_type(qlog_t);
  if (log == NULL) {
    return NULL;
  }
  log->n = strlen(thread_log->engine->time_buff);
  strcpy(log->buff, thread_log->engine->time_buff);
  qlist_add_tail(&log->list, thread_log->write);
  return log;
}
