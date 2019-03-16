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
typedef EpollEntry* handle_t;

class Poller {
public:
  Poller();

  virtual ~Poller();

  virtual int    Init(int size) = 0;
  virtual handle_t Add(fd_t fd, Event *event) = 0;
  virtual int    Del(handle_t) = 0;
  virtual int    ResetIn(handle_t) = 0; 
  virtual int    SetIn(handle_t) = 0; 
  virtual int    ResetOut(handle_t) = 0; 
  virtual int    SetOut(handle_t) = 0; 

  qid_t AddTimer(int timeout, Event *);
  void CancelTimer(qid_t);
  uint64_t NowMs() const {
    return clock_.NowMs();
  }

  void Loop();

protected:
  virtual int    Poll(int timeout) = 0;

  void updateTime();
  uint64_t executeTimers();
  void checkThread();

  // for load
  inline void adjustLoad(int cnt) {
    if (cnt > 0) {
      load_.add(cnt);
    } else if (cnt < 0) {
      load_.sub(-cnt);
    }
  }

  inline int getLoad() const {
    return load_.get();
  }

protected:  
  Clock clock_;
  struct TimerEntry {
    uint64_t expire;
    Event *event;
    qid_t id;

    TimerEntry(uint64_t ex, Event *e, qid_t i)
      : expire(ex),event(e), id(i) {}
  };
  typedef std::multimap<uint64_t, TimerEntry*> TimerMap;
  TimerMap timers_;
  typedef std::map<qid_t, TimerEntry*> TimerIdMap;
  TimerIdMap timer_ids_;
  qid_t max_id_;
  atomic_counter_t load_;
};

#endif // __QNODE_CORE_POLLER_H__
