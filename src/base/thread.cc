/*
 * Copyright (C) codedump
 */

#include <sys/prctl.h>
#include "base/thread.h"

Thread::Thread(const string& name)
  : tid_(0),
    name_(name) {
}

Thread::~Thread() {
}

int
Thread::Start(void* arg) {
  arg_ = arg;
  return pthread_create(&tid_, NULL, Thread::main, this);
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
  Thread *thread = (Thread*)arg;

  ::prctl(PR_SET_NAME, thread->name_.c_str());

  thread->Run(thread->arg_);

  return NULL;
}
