/*
 * Copyright (C) codedump
 */

#include "core/iothread.h"
#include "core/epoll.h"

IOThread::IOThread()
  : poller_(new Epoll()) {
  int rc = poller_->Init(1024);
  if (rc != kOK) {
    return;
  }

  // add mailbox signal fd into poller
  fd_t fd = mailbox_.Fd();
  handle_ = poller_->Add(fd, this);
  poller_->SetIn(handle_);
}

IOThread::~IOThread() {
  delete poller_;
}

void
IOThread::In() {
  Message* msg;

}

void
IOThread::Out() {
  // nothing to do
}

void
IOThread::Timeout() {
  // nothing to do
}

void
IOThread::Run(void *arg) {
  void(arg);

}
