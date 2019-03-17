/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_CORE_LOGTHREAD_H__
#define __QNODE_CORE_LOGTHREAD_H__

#include "base/singleton.h"
#include "core/iothread.h"
#include "core/message.h"

class Message;

class LogThread : public IOThread {
                  //public Singleton<LogThread> {
public:                    
  virtual ~LogThread();

  virtual void Process(Message*);

  virtual void Timeout();
private:
  LogThread();

private:
  string path_;
};

extern LogThread* gLogThread;

#endif  // __QNODE_CORE_LOGTHREAD_H__
