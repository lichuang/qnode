/*
 * Copyright (C) codedump
 */

#include "base/error.h"
#include "core/epoll.h"
#include "core/event.h"

Epoll::Epoll() 
  : size_(0),
    epoll_fd_(-1),
    load_(0) {
}

Epoll::~Epoll() {
  if (epoll_fd_ != -1) {
    close(epoll_fd_);
  }
  processRetired();
}

int 
Epoll::Init(int size) {
  size_ = size;
  ep_events_.reserve(size_);

  // in man 2 epoll:
  // epoll_create() creates an epoll(7) instance.  Since Linux 2.6.8, the size argument is ignored, but must be greater than zero
  epoll_fd_ = epoll_create(1);
  if (epoll_fd_ == -1) {
    return kError;
  }

  return kOK;
}

Handle
Epoll::Add(fd_t fd, Event *event) {
  CheckThread();

  EpollEntry *ee = new(std::nothrow)EpollEntry;
  alloc_assert(ee);
  memset(ee, 0, sizeof(EpollEntry));

  ee->fd = fd;
  ee->ev.events = events;
  ee->ev.data.ptr = ee;
  int ret = epoll_ctl(fd_, EPOLL_CTL_ADD, fd, &ee->ev);
  if (ret != 0) {
    delete ee;
    return NULL;
  }

  // change size to the max fd
  if (fd > size_) {
    size_ = fd;
    ep_events_.resize(size_);
  }
  ++load_;
  return ee;
}

int
Epoll::Del(Handle handle) {
  CheckThread();

  EpollEntry *ee = static_cast<EpollEntry *>(handle);
  int rc = epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, ee->fd, &ee->ev);
  ee->fd = kInvalidFd;
  retired_list_.push_back(ee);
  --load_;
  return rc;
}

int
Epoll::ResetIn(handle_t *handle) {
  CheckThread();

  EpollEntry *ee = static_cast<EpollEntry *>(handle);
  ee->ev.evnts &= ~EPOLLIN;
  int rc = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, ee->fd, &ee->ev);
  return rc;
}

int
Epoll::SetIn(handle_t *handle) {
  CheckThread();

  EpollEntry *ee = static_cast<EpollEntry *>(handle);
  ee->ev.evnts |= EPOLLIN;
  int rc = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, ee->fd, &ee->ev);
  return rc;
}

int
Epoll::ResetOut(handle_t *handle) {
  CheckThread();

  EpollEntry *ee = static_cast<EpollEntry *>(handle);
  ee->ev.evnts &= ~EPOLLOUT;
  int rc = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, ee->fd, &ee->ev);
  return rc;
}

int
Epoll::SetOut(handle_t *handle) {
  CheckThread();

  EpollEntry *ee = static_cast<EpollEntry *>(handle);
  ee->ev.evnts |= EPOLLOUT;
  int rc = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, ee->fd, &ee->ev);
  return rc;
}

int
Epoll::Poll(int timeout) {
  int num;
  struct epoll_event *epev;
  EpollEntry *ee;
  Event* event;

  if (load_ == 0) {
    if (timeout == 0) {
      return 0;
    }
  }

  num = epoll_wait(epoll_fd_, ep_events_.data(), size_, timeout);
  if (num <= 0) {
    return num;
  }

  for (i = 0; i < num; ++i) {
    epev = &(ep_events_[i]);
    ee = static_cast<EpollEntry *> epev->data.ptr;
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
    delete(*iter)
  }
  retired_list_.clear();
}
