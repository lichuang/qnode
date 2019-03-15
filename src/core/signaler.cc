/*
 * Copyright (C) codedump
 */

#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "base/net.h"
#include "core/const.h"
#include "core/signaler.h"

Signaler::Signaler()
  : wfd_(kInvalidFd),
    rfd_(kInvalidFd) {
  MakeFdPair(&wfd_, &rfd_);      
}

Signaler::~Signaler() {
  if (wfd_ != kInvalidFd) {
    Close(wfd_);
  }
  if (rfd_ != kInvalidFd) {
    Close(rfd_);
  }
}

void
Signaler::Send() {
  char dummy;
  while (true) {
    ssize_t nbytes = ::send(wfd_, &dummy, sizeof(dummy), 0);
    if (nbytes == -1 && errno == EINTR) {
      continue;
    }
    break;
  }
}

int
Signaler::Wait(int timeout) {
  struct pollfd pfd;
  pfd.fd = rfd_;
  pfd.events = POLLIN;
  return poll(&pfd, 1, timeout);
}

void
Signaler::Recv() {
  char dummy;
  ::recv(rfd_, &dummy, sizeof(dummy), 0);
}
