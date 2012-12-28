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

int __log_level = QLOG_DEBUG;

void qlog(int level, const char* file, long line, const char *fmt, ...) {
  if (__log_level < level) {
    return;
  }
  qlog_t log = {level, file, line};
  va_list  args;
  int n;
  char buff[500] = {' '};
  n = sprintf(buff, "[%s] %s:%d ", err_levels[log.level], log.file, log.line);
  va_start(args, fmt);
  vsprintf(buff + n, fmt, args);
  va_end(args);
  printf("%s\n", buff);
}
