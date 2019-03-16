/*
 * Copyright (C) codedump
 */

#include "base/errcode.h"
#include "core/iothread.h"
#include "core/epoll.h"

IOThread::IOThread(const string &name)
  : Thread(name),
    poller_(new Epoll()) {
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
  int rc = mailbox_.Recv(&msg, 0);

  while (rc == 0 || errno == EINTR) {
    if (rc == 0)  {
      msg->Process();
      delete msg;
    }
    rc = mailbox_.Recv(&msg, 0);
  }
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
IOThread::Process(Message *msg) {
}

void
IOThread::Run(void *arg) {
  arg = NULL;

  poller_->Loop();
}
