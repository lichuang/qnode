/*
 * Copyright (C) codedump
 */
#include "base/condition.h"
#include "base/time.h"

Condition::Condition() {
  pthread_cond_init(&cond_, NULL);      
}

Condition::~Condition() {
  ::pthread_cond_destroy(&cond_);
}

void
Condition::Wait(Mutex *mutex) {
  ::pthread_cond_wait(&cond_, &(mutex->mutex_));
}

bool
Condition::WaitUntil(Mutex* mutex, int timeout_ms) {
  struct timespec ts;
  MakeTimespec(timeout_ms, &ts);
  return ::pthread_cond_timedwait(&cond_, &(mutex->mutex_), &ts) == 0;
}

void
Condition::Notify() {
  ::pthread_cond_signal(&cond_);
}

void
Condition::NotifyAll() {
  ::pthread_cond_broadcast(&cond_);
}
