/*
 * See Copyright Notice in qnode.h
 */

#include "qassert.h"
#include "qmutex.h"

void qmutex_init(qmutex_t *mutex) {
  qassert(mutex);
  int rc = pthread_mutex_init(mutex, NULL);
  if (rc) {
    qassert(rc == 0);
  }
}

void qmutex_destroy(qmutex_t *mutex) {
  qassert(mutex);
  int rc = pthread_mutex_destroy(mutex);
  if (rc) {
    qassert(rc == 0);
  }
}

void qmutex_lock(qmutex_t *mutex) {
  qassert(mutex);
  int rc = pthread_mutex_lock(mutex);
  if (rc) {
    qassert(rc == 0);
  }
}

void qmutex_unlock(qmutex_t *mutex) {
  qassert(mutex);
  int rc = pthread_mutex_unlock(mutex);
  if (rc) {
    qassert(rc == 0);
  }
}
