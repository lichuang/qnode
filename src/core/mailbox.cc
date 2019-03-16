/*
 * Copyright (C) codedump
 */

#include "base/errcode.h"
#include "core/mailbox.h"
#include "core/message.h"

Mailbox::Mailbox()
  : active_(false) {
  pipe_.check_read();
}

Mailbox::~Mailbox() {
}

void Mailbox::Send(Message *msg) {
  sync_.Lock();
  pipe_.write(msg, false);
  bool ok = pipe_.flush();
  sync_.UnLock();
  if (!ok) {
    signaler_.Send();
  }
}

int
Mailbox::Recv(Message** msg, int timeout) {
  if (active_) {
    if (pipe_.read(msg)) {
      return kOK;
    }
    //  If there are no more commands available, switch into passive state.
    active_ = false;
  }
  //  Wait for signal from the command sender.
  int rc = signaler_.Wait(timeout);
  if (rc == -1) {
    return kError;
  }

  //  Receive the signal.
  rc = signaler_.RecvFailable();
  if (rc == -1) {
    return kError;
  }

  //  Switch into active state.
  active_ = true;
  pipe_.read(msg);
  return kOK;
}
