/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_BASE_CLOCK_H__
#define __QNODE_BASE_CLOCK_H__

#include <stdint.h>
#include "base/base.h"

class Clock {
public:
  Clock();

  uint64_t NowMs() const {
    return last_ms_;
  }

  void Update();

private:
  uint64_t last_ms_;
  DISALLOW_COPY_AND_ASSIGN(Clock);
};

#endif  // __QNODE_BASE_CLOCK_H__
