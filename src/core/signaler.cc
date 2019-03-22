/*
 * Copyright (C) codedump
 */

#include <errno.h>
#include <poll.h>
#include <unistd.h>
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
  uint64_t dummy;
  while (true) {
    ssize_t nbytes = ::write(wfd_, &dummy, sizeof(dummy));
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
  uint64_t dummy;
  ::read(rfd_, &dummy, sizeof(dummy));
}

int
Signaler::RecvFailable() {
  uint64_t dummy;
  return ::read(rfd_, &dummy, sizeof(dummy));
}
