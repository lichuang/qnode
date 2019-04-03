/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_SCRIPT_ACTOR_H__
#define __QNODE_SCRIPT_ACTOR_H__

#include "core/session.h"
#include "script/lua.h"

class LuaThread;

class Actor : public Session {
public:
  Actor(int fd, const string& addr)
    : Session(fd, addr) {
  }

  virtual ~Actor() {}

  virtual void OnWrite() {
  }

  virtual void OnRead();

  virtual void OnError(int err) {
  }

  void SetLuaThread(LuaThread *thread) {
    thread_ = thread;
  }

private:
  LuaThread *thread_;
};

class ActorFactory : public SessionFactory {
  virtual ~ActorFactory() {}

  virtual Session* Create(int fd, const string& addr) {
    return new Actor(fd, addr);
  }
};

#endif  // __QNODE_SCRIPT_ACTOR_H__
