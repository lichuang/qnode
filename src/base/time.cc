/*
 * Copyright (C) codedump
 */
#include "base/time.h"

static const uint64_t kUsecsPerMsec = 1000;
static const uint64_t kMsecsPerSec = 1000;
static const uint64_t kUsecsPerSec  = kUsecsPerMsec * kMsecsPerSec;

struct timespec *
MakeTimespec(int64_t ms, struct timespec *ts) {
  if (ms < 0) {
    ms = 0;
  }
  struct timeval tv;
  ::gettimeofday(&tv, 0);
  ts->tv_sec = tv.tv_sec + ms / kMsecsPerSec;
  ts->tv_nsec = tv.tv_usec * 1000 + (ms % 1000) * 1000000;
  if (ts->tv_nsec >= 1000000000) {
    ts->tv_nsec -= 1000000000;
    ts->tv_sec++;
  }
  return ts;
}

uint64_t
NowMs() {
  struct timeval tv;
  gettimeofday (&tv, NULL);
  return tv.tv_sec * kMsecsPerSec + tv.tv_usec;
}

void
NowMsString(string *ret) {
  char buf[512];
  struct timeval t;
  gettimeofday(&t, NULL);
  struct tm tim;
  ::localtime_r(&t.tv_sec, &tim);
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d.%03d",
    tim.tm_year + 1900, tim.tm_mon + 1, tim.tm_mday,
    tim.tm_hour, tim.tm_min, tim.tm_sec, (int)t.tv_usec / 1000);
  *ret = buf;
}
