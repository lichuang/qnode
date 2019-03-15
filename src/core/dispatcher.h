/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_CORE_DISPATCHER_H__
#define __QNODE_CORE_DISPATCHER_H__

#include <string>

using namespace std;

class Event;

class Dispatcher {
public:
  Dispatcher(const string& name)
    : name_(name)
      events_() {}

  virtual ~Dispatcher() {
    delete events_;
  }

  virtual int Init() = 0;
  virtual int Add(Event *event, int flags) = 0;
  virtual int Del(Event *event, int flags) = 0;
  virtual int Poll(int timeout_ms) = 0;

protected:
  string name_;
  int size_;
  Event* events_;
};

#endif  // __QNODE_CORE_DISPATCHER_H__
