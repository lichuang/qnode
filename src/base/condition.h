/*
 * Copyright (C) codedump
 */
#ifndef __QNODE_BASE_CONDITION_H__
#define __QNODE_BASE_CONDITION_H__

#include <pthread.h>
#include "base/base.h"
#include "base/mutex.h"

class Condition {
public:
  Condition();

  ~Condition();

  void Wait(Mutex *mutex);

  // return true if notified, else(include timeout) false
  bool WaitUntil(Mutex *, int timeout_ms);

  void Notify();
  void NotifyAll();

private:
  pthread_cond_t cond_;

  DISALLOW_COPY_AND_ASSIGN(Condition);
};

#endif  // __QNODE_BASE_CONDITION_H__
