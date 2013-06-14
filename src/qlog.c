/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qalloc.h"
#include "qassert.h"
#include "qlog.h"
#include "qlogger.h"
#include "qworker.h"
#include "qthread_log.h"

static const char* log_levels[] = {
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

static int log_level = QLOG_DEBUG;

static void log_init(qlog_t *log, int level, const char* file,
                     long line, const char *format, va_list args);

const char *
level_str(int level) {
  return log_levels[level];
}

static void
log_init(qlog_t *log, int level, const char* file,
         long line, const char *format, va_list args) {
  //log->file_len = strlen(file);
  strcpy(log->file, file);
  //log->fmt_len = strlen(format);
  strcpy(log->format, format);
  va_copy(log->args, args);
  log->level = level;
  log->line = line;
}

void
qlog(int level, const char* file, long line, const char *format, ...) {
  va_list args;

  if (logger == NULL) {
    return;
  }
  if (log_level < level) {
    return;
  }
  qlog_t *log = qthread_log_get();
  if (log == NULL) {
    return;
  }
  
  va_start(args, format);
  log_init(log, level, file, line, format, args);
  va_end(args);

  log->n = sprintf(log->buff, "%s %d", logger->time_buff, log->idx);
  log->n += sprintf(log->buff + log->n, " %s:%d] ", log->file, log->line);
  log->n += vsprintf(log->buff + log->n, log->format, args);
  log->buff[log->n++] = '\n';

#if 1  
  qlogger_add(log);
#else
  printf("%s\n", log->buff);
#endif
}
