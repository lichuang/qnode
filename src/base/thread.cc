/*
 * Copyright (C) codedump
 */

#include <sys/prctl.h>
#include "base/condition.h"
#include "base/mutex.h"
#include "base/thread.h"

struct threadStartEntry {
  Condition *cond;
  Thread *thread;
};

Thread::Thread(const string& name)
  : tid_(0),
    name_(name) {
}

Thread::~Thread() {
}

int
Thread::Start(void* arg) {
  arg_ = arg;
  Mutex mutex;
  Condition cond;
  threadStartEntry entry = {
    .cond = &cond,
    .thread = this
  };
  int ret = pthread_create(&tid_, NULL, Thread::main, &entry);

  // wait until thread start
  MutexGuard guard(&mutex);
  cond.Wait(&mutex);

  return ret;
}

void
Thread::Join() {
  if (tid_ != 0) {
    pthread_join(tid_, NULL);
    tid_ = 0;
  }
}

void*
Thread::main(void* arg) {
  threadStartEntry* entry = static_cast<threadStartEntry*>(arg);
  Condition *cond = entry->cond;
  Thread *thread = entry->thread;

  ::prctl(PR_SET_NAME, thread->name_.c_str());

  cond->Notify();

  thread->Run(thread->arg_);

  return NULL;
}
