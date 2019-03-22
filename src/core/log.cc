/*
 * Copyright (C) codedump
 */
#include <string.h>
#include <pthread.h>
#include "base/thread_local_storage.h"
#include "core/config.h"
#include "core/global.h"
#include "core/log.h"

extern void UpdateGlobalTime();

tls_key_t gThreadLogInfo;

int gLogLevel = LOG_DEBUG;

static const char* kLogLevelString[] = {
  "[C ",
  "[E ",
  "[W ",
  "[I ",
  "[D "
};

// per thread log info
struct threadLogInfo {
  char buffer[kLogBufferSize];
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

static void
destroyThreadLogInfo(void *arg) {
  threadLogInfo *i = static_cast<threadLogInfo*>(arg);
  delete i;
}

void
InitLog() {
  CreateTLSKey(&gThreadLogInfo, &destroyThreadLogInfo);
  UpdateGlobalTime();
}

void
Log(int level, const char* file, int line, const char *format, ...) {
  threadLogInfo *info = static_cast<threadLogInfo*>(GetTLS(gThreadLogInfo));
  if (info == NULL) {
    info = new threadLogInfo;
    CreateTLS(gThreadLogInfo, info);
  }

  char *p = &(info->buffer[0]);
  char *end = p + kLogBufferSize;
  int n;

  // log header

  // log level
  memcpy(p, kLogLevelString[level], 3);
  p += 3;
  // current time
  n = snprintf(p, end - p, gCurrentMsString.c_str());
  p += n;
  // pthread id
  n = snprintf(p, end - p, " %lu", pthread_self());
  p += n;
  // file:line
  n = snprintf(p, end - p, " %s:%d]", file, line);
  p += n;

  va_list ap;
  va_start(ap, format);
  n = snprintf(p, end - p, format, ap);
  va_end(ap);

  *(p + n) = '\n';
  *(p + n + 1) = '\0';
  printf("%s", info->buffer);
}
