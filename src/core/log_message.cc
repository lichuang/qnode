/*
 * Copyright (C) codedump
 */
#include "core/log_message.h"
#include "core/log_thread.h"

LogMessage::LogMessage(int level, const char* file,
                       int line, const char *format, ...)
  : Message(kLogMessage, 0, gLogThread),
    level_(level),
    file_(file),
    line_(line) {
}

