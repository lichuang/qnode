/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_CORE_LISTENER_H__
#define __QNODE_CORE_LISTENER_H__

#include <string>
#include "core/event.h"

using namespace std;

class AcceptorHandler;
class Poller;
class SessionFactory;

class Listener : public Event {
public:
  Listener(const string& addr, int port, Poller *poller,
           AcceptorHandler*, SessionFactory*);
  ~Listener();

  virtual void In();

  virtual void Out();

  virtual void Timeout();

  const char* String() const {
    return string_.c_str();
  }

private:
  string addr_;
  int port_;
  int fd_;
  Poller *poller_;
  AcceptorHandler *handler_;
  SessionFactory *factory_;
  string string_;
};

#endif  // __QNODE_CORE_LISTENER_H__
