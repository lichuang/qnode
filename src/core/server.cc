/*
 * Copyright (C) codedump
 */

#include "base/assert.h"
#include "core/accept_message.h"
#include "core/listener.h"
#include "core/io_thread.h"
#include "core/server.h"
#include "core/poller.h"
#include "core/epoll.h"

Server::Server(int worker)
  : index_(0) {
  poller_ = new Epoll();
  Assert(poller_ != NULL);
  Assert(worker > 0);
  int n;
  char buf[10];
  for (n = 0; n < worker; ++n) {
    snprintf(buf, sizeof(buf), "worker-%d", n + 1);
    IOThread *worker = new IOThread(buf);
    Assert(worker != NULL);
    workers_.push_back(worker);
  }
}

Server::~Server() {
  delete poller_;
  size_t n;
  for (n = 0; n < workers_.size(); ++n) {
    delete workers_[n];
  }
}

void
Server::OnAccept(Session* s) {
  index_ = (index_ + 1) % workers_.size();

  IOThread *worker = workers_[index_];
  AcceptMessage *msg = new AcceptMessage(s, worker->GetTid(), worker);
  worker->Send(msg);
}

void
Server::OnError(int err) {
}

void
Server::Listen(const string& addr, int port, SessionFactory* f) {
  Listener *listener = new Listener(addr, port, this, f);
  listeners_[listener->String()] = listener;
}

void
Server::Run() {
  poller_->Loop();
}
