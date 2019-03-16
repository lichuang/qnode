/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_CORE_MESSAGE_H__
#define __QNODE_CORE_MESSAGE_H__

#include "base/typedef.h"

class Message;

// virtual interface for Mailbox message handler
class MessageHandler {
public:
  virtual ~MessageHandler() {}

  virtual void Process(Message *) = 0;
};

// virtual interface for Mailbox message
class Message {
public:
  Message(tid_t tid, MessageHandler *handler)
    : tid_(tid), handler_(handler) {
  }

  virtual ~Message() {}

  void Process() {
    handler_->Process(this);
  }

protected:
  tid_t tid_;
  MessageHandler *handler_;
};

#endif // __QNODE_CORE_MESSAGE_H__
