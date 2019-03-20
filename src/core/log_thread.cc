/*
 * Copyright (C) codedump
 */

#include <stdio.h>
#include "core/log_message.h"
#include "core/log_thread.h"

static const string kLoggerThreadName = "LogThread";

LogThread::LogThread()
  : IOThread(kLoggerThreadName) {
}

LogThread::~LogThread() {
}

void
LogThread::Process(Message *msg) {
  LogMessage *lm = static_cast<LogMessage*>(msg);

  printf("msg: %s", lm->file_.c_str());
}

void
LogThread::Timeout() {
}
