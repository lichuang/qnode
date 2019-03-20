/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_CORE_ACCEPTOR_HANDLER_H__
#define __QNODE_CORE_ACCEPTOR_HANDLER_H__

class Session;

class AcceptorHandler {
public:
  virtual ~AcceptorHandler() {
  }

  virtual void OnAccept(Session*) = 0;
  virtual void OnError(int err) = 0;
};

#endif  // __QNODE_CORE_ACCEPTOR_HANDLER_H__
