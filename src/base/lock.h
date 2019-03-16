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

  bool LockRead();
  bool LockWrite();
  bool UnLock();
private:
  pthread_rwlock_t rw_lock_;
};

class ReadLockGuard {
public:
  ReadLockGuard (RWLock& lock) : rw_lock_(lock) {
    rw_lock_.LockRead();
  }
  ~ReadLockGuard () {
    rw_lock_.UnLock();
  }
private:
  RWLock& rw_lock_;
};

class SafeWriteLockGuard {
public:
  SafeWriteLockGuard (RWLock &lock) : rw_lock_(lock) {
    rw_lock_.LockWrite();
  }
  ~SafeWriteLockGuard() {
    rw_lock_.UnLock();
  }
private:
  RWLock& rw_lock_;
};

#endif  // __QNODE_BASE_LOCK_H__
