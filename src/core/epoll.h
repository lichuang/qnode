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
  fd_t fd;
  epoll_event ev;
  Event *event;
};

class Epoll : public Poller {
public:
  Epoll();

  virtual ~Epoll();

  virtual int    Init(int size);
  virtual handle_t Add(fd_t fd, Event *event);
  virtual int    Del(handle_t);
  virtual int    ResetIn(handle_t); 
  virtual int    SetIn(handle_t); 
  virtual int    ResetOut(handle_t); 
  virtual int    SetOut(handle_t); 
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
