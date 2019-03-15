/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_BASE_SOCKET_H__
#define __QNODE_BASE_SOCKET_H__

#include "base/base.h"

class BufferList;

class Socket {
public:
  Socket(int fd);
  ~Socket();

private:
  BufferList buffer_;

  DISALLOW_COPY_AND_ASSIGN(Socket);
};

#endif  // __QNODE_BASE_SOCKET_H__
