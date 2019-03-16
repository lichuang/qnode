/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_CORE_SIGNALER_H__
#define __QNODE_CORE_SIGNALER_H__

#include "base/base.h"
#include "core/typedef.h"

class Signaler {
public:
  Signaler();
  ~Signaler();

  fd_t Fd() const {
    return rfd_;
  }
  void Send();
  int Wait(int timeout);
  void Recv();

  int RecvFailable();
private:
  fd_t wfd_;
  fd_t rfd_;

  DISALLOW_COPY_AND_ASSIGN(Signaler);
};

#endif // __QNODE_CORE_SIGNALER_H__
