/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_CORE_EVENT_H__
#define __QNODE_CORE_EVENT_H__

#include "core/typedef.h"

class Dispatcher;

// event flags
static const int kEventIn       = 1 << 0;
static const int kEventOut      = 1 << 1;
static const int kEventError    = 1 << 2;
static const int kEventTimeout  = 1 << 3;

// virtual interface for notify events
class Event {
public:
  virtual ~Event() {}

  virtual void In() = 0;

  virtual void Out() = 0;

  virtual void Timeout(id_t id) = 0;
};

#endif  // __QNODE_CORE_EVENT_H__
