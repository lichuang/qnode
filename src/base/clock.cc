/*
 * Copyright (C) codedump
 */

#include <sys/time.h>
#include "base/clock.h"

static const uint64_t kUsecsPerMsec = 1000;
static const uint64_t kUsecsPerSec  = 1000000;
static const uint64_t kSecsPerUsec  = 1000;

Clock::Clock() {
  Update();
}

uint64_t Clock::NowMs() {
  return last_us_ / kUsecsPerMsec;
}

void
Clock::Update() {
  struct timeval tv;
  gettimeofday (&tv, NULL);
  last_ms_ = tv.tv_sec * kUsecsPerSec + tv.tv_usec;
}
