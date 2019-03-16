/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_CORE_EPOLL_H__
#define __QNODE_CORE_EPOLL_H__

#include <list>
#include <vector>
#include <sys/epoll.h>
#include "core/event.h"
#include "core/poller.h"

using namespace std;

struct EpollEntry {
  Fd fd;
  epoll_event ev;
  Event *event;
  int flags;
};

class Epoll {
public:
  Epoll();

  virtual ~Epoll();

  virtual int    Init(int size);
  virtual Handle Add(fd_t fd, Event *event);
  virtual int    Del(Handle);
  virtual int    ResetIn(Handle); 
  virtual int    SetIn(Handle); 
  virtual int    ResetOut(Handle); 
  virtual int    SetOut(Handle); 
  virtual int    Poll(int timeout);

private:  
  void   processRetired();

private:
  int    size_;
  int    epoll_fd_;
  vector<epoll_event> ep_events_;

  typedef list<EpollEntry*> EntryList;
  typedef list<EpollEntry*>::iterator EntryListIter;
  EntryList retired_list_;
};

#endif  // __QNODE_CORE_EPOLL_H__
