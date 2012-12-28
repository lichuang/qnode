/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qlog.h"

static const char* err_levels[] = {
  "",
  "emerg",
  "alert",
  "crit",
  "error",
  "warn",
  "notice",
  "info",
  "debug"
};

int g_log_level = QLOG_DEBUG;

static void init_log(qlog_t *log, int level, const char* file, long line, const char *format, va_list args) {
  strncpy(log->file, file, QMAX_LOG_SIZE - 1);
  strncpy(log->format, format, QMAX_FORMAT_SIZE - 1);
  va_copy(log->args, args);
  log->level = level;
  log->line = line;
}

void qlog(int level, const char* file, long line, const char *format, ...) {
  if (g_log_level < level) {
    return;
  }
  qlog_t log;
  va_list args;
  va_start(args, format);
  init_log(&log, level, file, line, format, args);
  va_end(args);

  int n;
  n = sprintf(log.buff, "[%s] %s:%d ", err_levels[log.level], log.file, log.line);
  vsprintf(log.buff + n, log.format, log.args);
  printf("%s\n", log.buff);
}
