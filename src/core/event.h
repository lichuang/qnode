/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_CORE_EVENT_H__
#define __QNODE_CORE_EVENT_H__

#include "core/typedef.h"

// virtual interface for notify events
class Event {
public:
  virtual ~Event() {}

  virtual void In() = 0;

  virtual void Out() = 0;

  virtual void Timeout(qid_t id) = 0;
};

#endif  // __QNODE_CORE_EVENT_H__
