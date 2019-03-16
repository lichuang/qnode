/*
 * Copyright (C) codedump
 */
#ifndef __QNODE_CORE_IOTHREAD_H__
#define __QNODE_CORE_IOTHREAD_H__

#include "base/thread.h"
#include "core/event.h"
#include "core/mailbox.h"

class Poller;

class IOThread : public Thread, public Event {
public:
  IOThread();
  virtual ~IOThread();

  virtual void In();

  virtual void Out();

  virtual void Timeout();

protected:
  virtual void Run(void *arg);

private:
  Poller *poller_;
  Mailbox mailbox_;
  Handle  handle_;
};

#endif  // __QNODE_CORE_IOTHREAD_H__
