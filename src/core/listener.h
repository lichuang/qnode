/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_CORE_LISTENER_H__
#define __QNODE_CORE_LISTENER_H__

#include <string>
#include "core/event.h"

using namespace std;

class AcceptorHandler;
class SessionFactory;

class Listener : public Event {
public:
  Listener(const string& addr, int port, AcceptorHandler*, SessionFactory*);
  ~Listener();

  virtual void In();

  virtual void Out();

  virtual void Timeout();

  string String();

private:
  string addr_;
  int port_;
  int fd_;
  AcceptorHandler *handler_;
  SessionFactory *factory_;
};

#endif  // __QNODE_CORE_LISTENER_H__
