/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_CORE_EVENT_H__
#define __QNODE_CORE_EVENT_H__

#include "core/poller.h"

// virtual interface for notify events
class Event {
public:
  Event() 
    : handle_(kInvalidHandle),
      timer_id_(kInvalidTimer) {
  }
  virtual ~Event() {}

  virtual void In() = 0;

  virtual void Out() = 0;

  virtual void Timeout() = 0;

  void SetTimerId(timer_id_t id) {
    timer_id_ = id;
  }

protected:
  handle_t handle_;
  timer_id_t timer_id_;
};

#endif  // __QNODE_CORE_EVENT_H__
