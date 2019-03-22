/*
 * Copyright (C) codedump
 */
#include "base/time.h"
#include "core/event.h"
#include "core/global.h"
#include "core/poller.h"

extern void UpdateGlobalTime();

Poller::Poller()
  : update_global_time_(false),
    max_timer_id_(0) {
}

Poller::~Poller() {
}

timer_id_t
Poller::AddTimer(int timeout, Event *event) {
  uint64_t expire = clock_.NowMs() + timeout;
  ++max_timer_id_;
  timer_id_t id = max_timer_id_;
  TimerEntry *entry = new TimerEntry(expire, event, id);
  timers_.insert(TimerMap::value_type(expire, entry));
  timer_ids_[id] = entry;
  event->SetTimerId(id);
  return id;
} 

void
Poller::CancelTimer(timer_id_t id) {
  TimerIdMap::iterator iter = timer_ids_.find(id);
  if (iter == timer_ids_.end()) {
    return;
  }
  TimerEntry *entry = iter->second;
  uint64_t expire = entry->expire;
  TimerMap::iterator pos;
  for (pos = timers_.lower_bound(expire);
       pos != timers_.upper_bound(expire); ++pos) {
    if (pos->second->id == id) {
      break;
    }
  }
  timers_.erase(pos);
  timer_ids_.erase(id);
  delete entry;
}

uint64_t
Poller::executeTimers() {
  if (timers_.empty()) {
    return 0;
  }
  uint64_t current = clock_.NowMs();
  uint64_t res = 0;
  TimerIdMap::iterator iter = timers_.begin();
  TimerIdMap::iterator begin = timers_.begin();
  TimerIdMap::iterator end = timers_.end();
  for (; iter != end; ++iter) {
    if (iter->first > current) {
      res = iter->first - current;
      break;
    }

    iter->second->event->Timeout();
  }
  timers_.erase(begin, iter);

  return res;
}

void
Poller::updateTime() {
  clock_.Update();
  if (update_global_time_) {
    UpdateGlobalTime();
  }
}

void
Poller::Loop() {
  int timeout;

  while (true) {
    timeout = executeTimers();
    if (timeout) {
      if (getLoad() == 0) {
        continue;
      }
    }

    Poll(timeout);
  }
}

void
Poller::checkThread() {
}
