/*
 * Copyright (C) codedump
 */

#include <sys/prctl.h>
#include "base/condition.h"
#include "base/mutex.h"
#include "base/thread.h"
#include "base/thread_local_storage.h"

tls_key_t gThreadInfo;

struct threadStartEntry {
  Condition *cond;
  Thread *thread;
};

static void
destroyThreadInfo(void *arg) {
  threadInfo *i = static_cast<threadInfo*>(arg);
  delete i;
}

Thread::Thread(const string& name)
  : tid_(0),
    name_(name) {
  CreateTLSKey(&gThreadInfo, &destroyThreadInfo);
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

const char*
CurrentThreadName() {
  return CurrentThreadInfo()->name.c_str();
}

threadInfo* CurrentThreadInfo() {
  threadInfo *info = static_cast<threadInfo*>(GetTLS(gThreadInfo));
  if (info == NULL) {
    info = new threadInfo;

    char buffer[100];
    pthread_getname_np(pthread_self(), buffer, sizeof(buffer));
    info->name = buffer;

    CreateTLS(gThreadInfo, info);
  }

  return info;
}
