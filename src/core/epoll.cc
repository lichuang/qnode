/*
 * Copyright (C) codedump
 */

#include "base/error.h"
#include "core/epoll.h"
#include "core/event.h"

Epoll::Epoll(int size) 
  : Dispatcher("epoll", size),
    size_(size),
    fd_(-1) {
}

Epoll::~Epoll() {
  if (events_ != NULL) {
    free(events_);
  }
  if (fd_ != -1) {
    close(fd);
  }
}

int 
Epoll::Init() {
  events_ = calloc(sizeof(struct epoll_event) * size_);
  if (events_ == NULL) {
    return kError;
  }

  fd_ = epoll_create(1024);
  if (fd_ == -1) {
    return kError;
  }

  return kOK;
}

int
Epoll::Add(Event *event, int flags) {
  int events = 0, op;
  struct epoll_event  event;

  memset(&event, 0, sizeof(struct epoll_event));

  if (events_[fd] == NULL) {
    op = EPOLL_CTL_ADD;
  } else {
    op = EPOLL_CTL_MOD;
  }

  if (flags & kEventRead) {
    events |= EPOLLIN;
  }
  if (flags & kEventWrite) {
    events |= EPOLLOUT;
  }

  event.events = events;
  event.data.fd = fd;
  if (epoll_ctl(epoll->fd, op, fd, &event) == -1) {
    qerror("epoll_ctl error: %s", strerror(errno));
    return kError;
  }

  return kOK;
}

int
Epoll::Del(Event *event, int flags) {
}

int
Epoll::Poll(int timeout, list<Event*> active) {
  int num, i, flags, fd;
  struct epoll_event *epev;
  Event* event;

  num = epoll_wait(fd_, &(ep_events_[0]), size_, timeout);
  if (num <= 0) {
    return num;
  }

  for (i = 0; i < num; ++i) {
    flags = 0;
    epev = ep_events_ + i;
    fd = epev->data.fd;

    event = events_[fd];
    if (epev->events & EPOLLIN) {
      flags |= kEventRead;
    }
    if (epev->events & EPOLLOUT) {
      flags |= kEventWrite;
    }
    event->SetFlags(flags);
    active->push_back(event);
  }

  return active->size();
}
