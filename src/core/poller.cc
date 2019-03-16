/*
 * Copyright (C) codedump
 */

#include "core/poller.h"

Poller::Poller()
  : max_id_(0) {
}

Poller::~Poller() {
}

id_t
Poller::AddTimer(int timeout, Event *event) {
  uint64_t expire = clock_.NowMs() + timeout;
  TimerEntry *entry = new TimerEntry(expire, event, ++max_id_);
  timers_.insert(TimerMap::value_type(expire, entry));
  timer_ids_[entry->id] = entry;
  return entry->id;
} 

void
Poller::CancelTimer(id_t id) {
  TimerIdMap::iterator iter = timer_ids_[id];
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
  for (; iter < end; ++iter) {
    if (iter->first > current) {
      res = it->first - current;
      break;
    }

    iter->second->event->Timeout(iter->second->id);
  }
  timers_.erase(begin, iter);

  return res;
}

void
Poller::updateTime() {
  clock_.Update();
}

void
Poller::checkThread() {
}
