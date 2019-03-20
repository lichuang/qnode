/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_CORE_DATA_HANDLER_H__
#define __QNODE_CORE_DATA_HANDLER_H__

// virtual interface for socket data handler
class DataHandler {
public:
  virtual ~DataHandler() {
  }

  virtual void OnWrite() = 0;
  virtual void OnRead() = 0;
  virtual void OnError(int err) = 0;
};

#endif  // __QNODE_CORE_DATA_HANDLER_H__
