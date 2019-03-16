/*
 * Copyright (C) codedump
 */
#ifndef __QNODE_CORE_POLLER_H__
#define __QNODE_CORE_POLLER_H__

#include <map>
#include "base/atomic.h"
#include "base/clock.h"
#include "core/event.h"

// epoll
class EpollEntry;
typedef EpollEntry* Handle;

class Poller {
public:
  Poller();

  virtual ~Poller();

  virtual int    Init(int size) = 0;
  virtual Handle Add(fd_t fd, Event *event) = 0;
  virtual int    Del(Handle) = 0;
  virtual int    ResetIn(Handle) = 0; 
  virtual int    SetIn(Handle) = 0; 
  virtual int    ResetOut(Handle) = 0; 
  virtual int    SetOut(Handle) = 0; 
  virtual int    Poll(int timeout) = 0;

  id_t AddTimer(int timeout, Event *);
  void CancelTimer(id_t);
  uint64_t NowMs() const {
    return clock_.NowMs();
  }

protected:
  void updateTime();
  uint64_t executeTimers();
  void checkThread();

protected:  
  Clock clock_;
  struct TimerEntry {
    uint64_t expire;
    Event *event;
    id_t id;

    TimerEntry(uint64_t ex, Event *e, id_t i)
      : expire(ex),event(e), id(i) {}
  };
  typedef std::multimap<uint64_t, TimerEntry*> TimerMap;
  TimerMap timers_;
  typedef std::map<id_t, TimerEntry*> TimerIdMap;
  TimerIdMap timer_ids_;
  id_t max_id_;
  atomic_counter_t load_;
};

#endif // __QNODE_CORE_POLLER_H__
