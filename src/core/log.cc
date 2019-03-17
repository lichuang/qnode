/*
 * Copyright (C) codedump
 */
#include "core/log.h"
#include "core/log_message.h"
#include "core/log_thread.h"

int gLogLevel;

void
SetLogLevel(int level) {
  gLogLevel = level;
}

void
Log(int level, const char* file, int line, const char *format, ...) {
  if (gLogThread == NULL) {
    return;
  }
  // TODO: use object pool to cache log message?
  LogMessage *msg = new LogMessage(level, file, line, format);
  gLogThread->Send(msg);
}
