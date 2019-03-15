/*
 * Copyright (C) codedump
 */
#ifndef __QNODE_BASE_LOCK_H__
#define __QNODE_BASE_LOCK_H__

#include <pthread.h>

class RWLock {
public :
  RWLock();
  ~RWLock();

  bool rdlock();
  bool wrlock();
  bool unlock();
private:
  pthread_rwlock_t rw_lock_;
};

class SafeReadLockGuard {
public:
  SafeReadLockGuard (RWLock& lock) : rw_lock_(lock) {
    rw_lock_.rdlock();
  }
  ~SafeReadLockGuard () {
    rw_lock_.unlock();
  }
private:
  RWLock& rw_lock_;
};

class SafeWriteLockGuard {
public:
  SafeWriteLockGuard (RWLock &lock) : rw_lock_(lock) {
    rw_lock_.wrlock();
  }
  ~SafeWriteLockGuard() {
    rw_lock_.unlock();
  }
protected:
  RWLock& rw_lock_;
};

#endif  // __QNODE_BASE_LOCK_H__
