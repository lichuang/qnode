/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_CORE_SESSION_H__
#define __QNODE_CORE_SESSION_H__

#include "core/data_handler.h"
#include "socket.h"

class Session : public DataHandler {
public:
  Session(int fd)
    : socket_(new Socket(fd, this)) {
  }

  virtual ~Session() {
    delete socket_;
  }

  virtual void OnWrite() = 0;
  virtual void OnRead() = 0;
  virtual void OnError(int err) = 0;

  Socket* GetSocket() {
    return socket_;
  }
protected:
  Socket *socket_;
};

class SessionFactory {
public:
  virtual ~SessionFactory() {
  }

  virtual Session* Create(int fd) = 0;
};

#endif  // __QNODE_CORE_SESSION_H__
