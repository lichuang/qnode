/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QMUTEX_H__
#define __QMUTEX_H__

#include <pthread.h>

typedef pthread_mutex_t qmutex_t;
typedef pthread_spinlock_t qspinlock_t;

#define qmutex_init(mutex)          pthread_mutex_init((mutex), NULL)
#define qmutex_destroy(mutex)       pthread_mutex_destroy((mutex))
#define qmutex_lock(mutex)          pthread_mutex_lock((mutex))
#define qmutex_unlock(mutex)        pthread_mutex_unlock((mutex))

#define qspinlock_init(spinlock)    pthread_spin_init((spinlock), 0)
#define qspinlock_destroy(spinlock) pthread_spin_destroy((spinlock))
#define qspinlock_lock(spinlock)    pthread_spin_lock((spinlock))
#define qspinlock_unlock(spinlock)  pthread_spin_unlock((spinlock))

#endif  /* __QMUTEX_H__ */
