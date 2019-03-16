/*
 * Copyright (C) codedump
 */

#include <stdio.h>
#include <sys/time.h>
#include "base/clock.h"

static const uint64_t kUsecsPerMsec = 1000;
static const uint64_t kUsecsPerSec  = 1000000;

Clock::Clock() {
  Update();
}

void
Clock::Update() {
  struct timeval tv;
  gettimeofday (&tv, NULL);
  last_ms_ = tv.tv_sec * kUsecsPerSec + tv.tv_usec;
}
