/*
 * Copyright (C) codedump
 */

#include <string.h>
#include "base/errcode.h"
#include "base/net.h"
#include "core/config.h"
#include "core/const.h"
#include "core/epoll.h"
#include "core/event.h"

Epoll::Epoll() 
  : size_(0),
    epoll_fd_(kInvalidFd) {
}

Epoll::~Epoll() {
  if (epoll_fd_ != kInvalidFd) {
    Close(epoll_fd_);
  }
  processRetired();
}

int 
Epoll::Init(int size) {
  size_ = size;
  ep_events_.reserve(size_);

  // in man 2 epoll:
  // epoll_create() creates an epoll(7) instance.  
  // Since Linux 2.6.8, the size argument is ignored, but must be greater than zero
  epoll_fd_ = epoll_create(1);
  if (epoll_fd_ == -1) {
    return kError;
  }

  return kOK;
}

handle_t
Epoll::Add(fd_t fd, Event *event) {
  checkThread();

  EpollEntry *ee = new(std::nothrow)EpollEntry;
  memset(ee, 0, sizeof(EpollEntry));

  ee->fd = fd;
  ee->event = event;
  ee->ev.data.ptr = ee;
  int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ee->ev);
  if (ret != 0) {
    delete ee;
    return NULL;
  }

  // change size to the max fd
  if (fd > size_) {
    size_ = fd + kIncrPollSize;
    ep_events_.resize(size_);
  }
  load_.add(1);
  return ee;
}

int
Epoll::Del(handle_t handle) {
  checkThread();

  EpollEntry *ee = static_cast<EpollEntry *>(handle);
  int rc = epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, ee->fd, &ee->ev);
  ee->fd = kInvalidFd;
  retired_list_.push_back(ee);
  load_.add(-1);
  return rc;
}

int
Epoll::ResetIn(handle_t handle) {
  checkThread();

  EpollEntry *ee = static_cast<EpollEntry *>(handle);
  ee->ev.events &= ~EPOLLIN;
  int rc = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, ee->fd, &ee->ev);
  return rc;
}

int
Epoll::SetIn(handle_t handle) {
  checkThread();

  EpollEntry *ee = static_cast<EpollEntry *>(handle);
  ee->ev.events |= EPOLLIN;
  int rc = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, ee->fd, &ee->ev);
  return rc;
}

int
Epoll::ResetOut(handle_t handle) {
  checkThread();

  EpollEntry *ee = static_cast<EpollEntry *>(handle);
  ee->ev.events &= ~EPOLLOUT;
  int rc = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, ee->fd, &ee->ev);
  return rc;
}

int
Epoll::SetOut(handle_t handle) {
  checkThread();

  EpollEntry *ee = static_cast<EpollEntry *>(handle);
  ee->ev.events |= EPOLLOUT;
  int rc = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, ee->fd, &ee->ev);
  return rc;
}

int
Epoll::Poll(int timeout) {
  int num, i;
  struct epoll_event *epev;
  EpollEntry *ee;
  Event* event;

  num = epoll_wait(epoll_fd_, ep_events_.data(), size_, timeout);
  if (num <= 0) {
    return num;
  }

  for (i = 0; i < num; ++i) {
    epev = &(ep_events_[i]);
    ee = static_cast<EpollEntry *>(epev->data.ptr);
    event = ee->event;

    // is something error happen?
    if (ee->fd == kInvalidFd) {
      continue;
    }
    if (epev->events & (EPOLLERR | EPOLLHUP)) {
      event->In();
    }

    // is something in happen?
    if (ee->fd == kInvalidFd) {
      continue;
    }
    if (epev->events & EPOLLIN) {
      event->In();
    }

    // is something out happen?
    if (ee->fd == kInvalidFd) {
      continue;
    }
    if (epev->events & EPOLLOUT) {
      event->Out();
    }
  }

  processRetired();

  return num;
}

void
Epoll::processRetired() {
  // destroy retired events
  for (EntryListIter iter = retired_list_.begin();
       iter != retired_list_.end(); ++iter) {
    delete(*iter);
  }
  retired_list_.clear();
}
