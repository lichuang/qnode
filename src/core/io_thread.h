/*
 * Copyright (C) codedump
 */
#ifndef __QNODE_CORE_IO_THREAD_H__
#define __QNODE_CORE_IO_THREAD_H__

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

  void Send(Message *msg);
protected:
  virtual void Run(void *arg);

protected:
  Poller *poller_;
  Mailbox mailbox_;

  static pthread_once_t once_;
  static void initIothreadOnceResource();
  DISALLOW_COPY_AND_ASSIGN(IOThread);
};


#endif  // __QNODE_CORE_IO_THREAD_H__
