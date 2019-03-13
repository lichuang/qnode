/*
 * Copyright (C) codedump
 */
#ifndef __QNODE_BASE_THREAD__
#define __QNODE_BASE_THREAD__

#include <string>
#include <pthread.h>
#include "base/runnable.h"

using namespace std;

class Thread {
public:
  Thread(const string& name);

  int Start(Runnable *r, void *arg);

  void Join();

private:
  static void* main(void *arg);

protected:
  pthread_t tid_;
  Runnable *runnable_;
  string name_;
  void *arg_;
};

#endif  // __QNODE_BASE_THREAD__
