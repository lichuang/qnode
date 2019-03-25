/*
 * Copyright (C) codedump
 */
#include <string.h>
#include <pthread.h>
#include "base/thread.h"
#include "base/thread_local_storage.h"
#include "core/config.h"
#include "core/global.h"
#include "core/log.h"

extern void UpdateGlobalTime();

int gLogLevel = LOG_DEBUG;

static const char* kLogLevelString[] = {
  "C",
  "E",
  "W",
  "I",
  "D"
};

// global thread info
struct globalThreadInfo{
  int fd;
  string path;
};

void
SetLogLevel(int level) {
  gLogLevel = level;
}

void
InitLog() {
  UpdateGlobalTime();
}

void
Log(int level, const char* file, int line, const char *fmt, ...) {
  threadInfo *info = CurrentThreadInfo();
  char *p = &(info->buffer[0]);
  char *end = p + kLogBufferSize;
  int n;

  // log header
  n = snprintf(p, end - p, "[%s %s %s %s:%d]", 
    kLogLevelString[level], gCurrentMsString.c_str(),
    info->name.c_str(), file, line);
  p += n;

  va_list ap;
  va_start(ap, fmt);
  n = vsnprintf(p, end - p, fmt, ap);
  va_end(ap);

  *(p + n) = '\n';
  *(p + n + 1) = '\0';

  printf("%s", info->buffer);
}
