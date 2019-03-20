/*
 * Copyright (C) codedump
 */

#include "base/net.h"
#include "core/data_handler.h"
#include "core/log.h"
#include "core/socket.h"

Socket::Socket(int fd, DataHandler* h)
  : fd_(fd),
    handler_(h),
    poller_(NULL),
    is_writable_(false) {
}

Socket::~Socket() {
  CloseSocket();
}

void
Socket::SetPoller(Poller *poller) {
  poller_ = poller;
  handle_ = poller->Add(fd_, this);
  poller->SetIn(handle_);
}

void
Socket::CloseSocket() {
  poller_->Del(handle_);
  Close(fd_);
}

void
Socket::In() {
  while (true) {
    int n = Recv(fd_, &read_list_, &error_);
    if (n < 0) {
      if (handler_) {
        handler_->OnError(error_);
      }
      CloseSocket();
      break;
    } else {
      read_list_.WriteAdvance(n);
      if (error_ == kAgain) {
        break;
      }
    }
  }

  if (handler_) {
    handler_->OnRead();
  }
}

void
Socket::Out() {
  while (true) {
    if (write_list_.Empty()) {
      is_writable_ = true;
      break;
    }

    int n = Send(fd_, &write_list_, &error_);
    if (n < 0) {
      CloseSocket();
      if (handler_) {
        handler_->OnError(error_);
      }
      break;
    } else {
      write_list_.ReadAdvance(n);
      if (error_ == kAgain) {
        is_writable_ = false;
        break;
      }
    }
  }

  if (handler_) {
    handler_->OnWrite();
  }
}

void
Socket::Timeout() {
}

void
Socket::Write(const char* from, size_t n) {
  write_list_.Write(from, n);
  if (is_writable_) {
    poller_->SetOut(handle_);
  }
}

void
Socket::Read(char* to, size_t n) {
  read_list_.Read(to, n);
}
