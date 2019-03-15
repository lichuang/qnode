/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_CORE_EPOLL_H__
#define __QNODE_CORE_EPOLL_H__

#include <sys/epoll.h>

class Epoll : public Dispatcher {
public:
  Epoll();

  virtual ~Epoll();

  virtual int Init() = 0;
  virtual int Add(Event *event, int flags) = 0;
  virtual int Del(Event *event, int flags) = 0;
  virtual int Poll(int timeout_ms) = 0;

private:
  int    size_;
  int    fd_;
  struct epoll_event *ep_events_;
};

#endif  // __QNODE_CORE_EPOLL_H__
