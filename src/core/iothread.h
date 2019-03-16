/*
 * Copyright (C) codedump
 */
#ifndef __QNODE_CORE_IOTHREAD_H__
#define __QNODE_CORE_IOTHREAD_H__

#include <string>
#include "base/thread.h"
#include "core/event.h"
#include "core/mailbox.h"
#include "core/message.h"
#include "core/poller.h"

using namespace std;

class IOThread 
  : public Thread,
    public Event,
    public MessageHandler {
public:
  IOThread(const string& name);
  virtual ~IOThread();

  virtual void In();

  virtual void Out();

  virtual void Timeout();

  virtual void Process(Message*);

  Poller* GetPoller() {
    return poller_;
  }

protected:
  virtual void Run(void *arg);

private:
  Poller *poller_;
  Mailbox mailbox_;
  handle_t  handle_;
};

#endif  // __QNODE_CORE_IOTHREAD_H__
