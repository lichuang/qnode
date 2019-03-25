/*
 * Copyright (C) codedump
 */

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "base/assert.h"
#include "base/errcode.h"
#include "base/net.h"
#include "base/string.h"
#include "core/acceptor_handler.h"
#include "core/config.h"
#include "core/listener.h"
#include "core/log.h"
#include "core/session.h"

Listener::Listener(const string& addr, int port, Poller* poller,
                   AcceptorHandler *h, SessionFactory *f)
  : addr_(addr),
    port_(port),
    poller_(poller),
    handler_(h),
    factory_(f) {
  int error;      
  fd_ = Listen(addr, port, kBacklog, &error);
  Assert(fd > 0);
  handle_ = poller_->Add(fd_, this, kEventRead);
  Stringf(&string_, "%s:%d", addr.c_str(), port);
}

Listener::~Listener() {
  Close(fd_);
}

void
Listener::In() {
  int error;
  string addr;

  while (true) {
    int fd = Accept(fd_, &addr, &error);
    if (fd > 0) {
      Session *s = factory_->Create(fd, addr);
      handler_->OnAccept(s);
    } else if (fd == kOK) {
      break;
    } else {
      handler_->OnError(error);
      Errorf("accept connection error: %s", strerror(error));
    }
  }
}

void
Listener::Out() {
  Assert(false);
}

void
Listener::Timeout() {
  Assert(false);
}
