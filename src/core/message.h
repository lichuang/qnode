/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_CORE_MESSAGE_H__
#define __QNODE_CORE_MESSAGE_H__

// virtual interface for Mailbox message
class Message {
public:
  virtual ~Message() {};
  virtual int Process() = 0;
};

#endif // __QNODE_CORE_MESSAGE_H__
