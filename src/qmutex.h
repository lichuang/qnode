/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QMUTEX_H__
#define __QMUTEX_H__

#include <pthread.h>

typedef pthread_mutex_t qmutex_t;

void qmutex_init(qmutex_t *mutex);
void qmutex_destroy(qmutex_t *mutex);
void qmutex_lock(qmutex_t *mutex);
void qmutex_unlock(qmutex_t *mutex);

#endif  /* __QMUTEX_H__ */
