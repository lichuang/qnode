/*
 * Copyright (C) codedump
 */
#ifndef __QNODE_BASE_THREAD__
#define __QNODE_BASE_THREAD__

#include <string>
#include <pthread.h>
#include "base/typedef.h"

using namespace std;

class Thread {
public:
  Thread(const string& name);

  virtual ~Thread();

  int Start(void *arg);

  void Join();

  tid_t Tid() const {
    return tid_;
  }
protected:
  virtual void Run(void* arg) = 0;

private:
  static void* main(void *arg);

protected:
  tid_t tid_;
  string name_;
  void *arg_;
};

#endif  // __QNODE_BASE_THREAD__
