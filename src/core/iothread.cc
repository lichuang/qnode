/*
 * Copyright (C) codedump
 */

#include "base/buffer.h"
#include "base/errcode.h"
#include "base/global.h"
#include "base/object_pool.h"
#include "base/thread_local_storage.h"
#include "core/iothread.h"
#include "core/epoll.h"

tls_key_t gBufferPoolKey;

static void
destroyBufferPool(void *arg) {
  ObjectPool<Buffer> *pool = static_cast<ObjectPool<Buffer>*>(arg);
  delete pool;
}

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
IOThread::Send(Message *msg) {
  mailbox_.Send(msg);
}

void
IOThread::Run(void *arg) {
  arg = NULL;

  // create buffer list object pool
  ObjectPool<Buffer> *buf_list = new ObjectPool<Buffer>();
  CreateTLS(&gBufferPoolKey, buf_list, destroyBufferPool);

  poller_->Loop();
}
