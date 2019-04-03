/*
 * Copyright (C) codedump
 */

#include "base/assert.h"
#include "base/time.h"
#include "base/string.h"
#include "core/accept_message.h"
#include "core/io_thread.h"
#include "core/listener.h"
#include "core/log.h"
#include "core/server.h"
#include "core/poller.h"
#include "core/epoll.h"
#include "script/script_io_thread.h"

volatile uint64_t gCurrentMs;
string   gCurrentMsString;

void
UpdateGlobalTime() {
  gCurrentMs = NowMs();
  string ret;
  NowMsString(&ret);
  gCurrentMsString = ret;
}

Server::Server(int worker)
  : index_(0),
    poller_(new Epoll()) {
  Assert(poller_ != NULL);
  Assert(worker > 0);
  poller_->Init(1024);
  poller_->SetUpdateGlobalTime();

  int n;
  for (n = 0; n < worker; ++n) {
    string name;
    Stringf(&name, "lua-worker-%d", n + 1);
    ScriptIOThread *worker = new ScriptIOThread(name);
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
  Listener *listener = new Listener(addr, port, poller_, this, f);
  Assert(listener != NULL);
  listeners_[listener->String()] = listener;
  Infof("listen at %s", listener->String());
}

void
Server::Run() {
  poller_->Loop();
}
