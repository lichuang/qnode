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

  virtual ~Thread();

  int Start();

  void Join();

protected:
  virtual void Run();

private:
  static void* main(void *arg);

protected:
  pthread_t tid_;
  string name_;
  void *arg_;
};

#endif  // __QNODE_BASE_THREAD__
