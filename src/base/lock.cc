/*
 * Copyright (C) codedump
 */

#include "base/lock.h"

RWLock::RWLock() {
  pthread_rwlock_init(&rw_lock_, NULL);
}

RWLock::~RWLock() {
  pthread_rwlock_destroy(&rw_lock_);
}

bool RWLock::rdlock() {
  return pthread_rwlock_rdlock(&rw_lock_) == 0;
}

bool RWLock::wrlock() {
  return pthread_rwlock_wrlock(&rw_lock_) == 0;
}

bool RWLock::unlock() {
  return pthread_rwlock_unlock(&rw_lock_) == 0;
}
