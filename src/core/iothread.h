/*
 * Copyright (C) codedump
 */
#ifndef __QNODE_CORE_IOTHREAD_H__
#define __QNODE_CORE_IOTHREAD_H__

#include "base/thread.h"
#include "core/event.h"

class Dispatcher;

class IOThread : public Thread, public Event {
public:
  IOThread();
  virtual ~IOThread();

  virtual void In();

  virtual void Out();

  virtual void Timeout();

protected:
  virtual void Run();

private:
  Dispatcher* dispatcher_;
};

#endif  // __QNODE_CORE_IOTHREAD_H__
