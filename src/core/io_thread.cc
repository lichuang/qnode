/*
 * Copyright (C) codedump
 */

#include "base/buffer.h"
#include "base/errcode.h"
#include "base/global.h"
#include "base/object_pool.h"
#include "base/thread_local_storage.h"
#include "core/accept_message.h"
#include "core/epoll.h"
#include "core/io_thread.h"
#include "core/session.h"
#include "core/socket.h"

pthread_once_t IOThread::once_ = PTHREAD_ONCE_INIT;
tls_key_t gBufferPoolKey;

static void
destroyBufferPool(void *arg) {
  ObjectPool<Buffer> *pool = static_cast<ObjectPool<Buffer>*>(arg);
  delete pool;
}

void IOThread::initIothreadOnceResource() {
  CreateTLSKey(&gBufferPoolKey, &destroyBufferPool);
}

IOThread::IOThread(const string &name)
  : Thread(name),
    poller_(new Epoll()) {
  pthread_once(&once_, &IOThread::initIothreadOnceResource);

  // create buffer list object pool
  ObjectPool<Buffer> *buf_list = new ObjectPool<Buffer>();
  CreateTLS(gBufferPoolKey, buf_list);

  int rc = poller_->Init(1024);
  if (rc != kOK) {
    return;
  }

  // add mailbox signal fd into poller
  fd_t fd = mailbox_.Fd();
  handle_ = poller_->Add(fd, this, kEventRead);

  Start(NULL);
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
  int type = msg->Type();

  if (type == kAcceptMessage) {
    AcceptMessage* am = static_cast<AcceptMessage*>(msg);
    Session* session = am->GetSession();
    Socket* socket = session->GetSocket();
    socket->SetPoller(poller_);
  }
}

void
IOThread::Send(Message *msg) {
  mailbox_.Send(msg);
}

void
IOThread::Run(void *arg) {
  arg = NULL;

  poller_->Loop();
}
