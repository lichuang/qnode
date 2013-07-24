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

//qthread_log_t* thread_log[QMAX_WORKER] = {NULL};
extern pthread_key_t thread_log_key;

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
  if (pthread_setspecific(thread_log_key, thread_log) < 0) {
    qfree(thread_log);
    return NULL;
  }
  qassert(pthread_getspecific(thread_log_key) != NULL);
  thread_log->idx = idx;

  return thread_log;
}
