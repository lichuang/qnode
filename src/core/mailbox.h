/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_CORE_MAILBOX_H__
#define __QNODE_CORE_MAILBOX_H__

#include "base/base.h"
#include "base/ypipe.h"
#include "core/signaler.h"

class Message;

class Mailbox {
public:
  Mailbox();
  ~Mailbox();

  fd_t Fd() const {
    return signaler_.Fd();
  }
  void Send(Message *);
  int  Recv(Message **, int timeout);

private:
  typedef ypipe_t<Message*, command_pipe_granularity> cpipe_t;
  cpipe_t pipe_;

  Signaler signaler_;

  bool active_;

  DISALLOW_COPY_AND_ASSIGN(Mailbox);
};

#endif // __QNODE_CORE_MAILBOX_H__
