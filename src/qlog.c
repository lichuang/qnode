/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qlog.h"
#include "qlog_thread.h"
#include "qthread_log.h"

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
  //log->file_len = strlen(file);
  strcpy(log->file, file);
  //log->fmt_len = strlen(format);
  strcpy(log->format, format);
  va_copy(log->args, args);
  log->level = level;
  log->line = line;
}

void qlog(int level, const char* file, long line, const char *format, ...) {
  if (g_log_level < level) {
    return;
  }
  qlog_t *log = qthread_log_get();
  if (log == NULL) {
    return;
  }
  va_list args;
  va_start(args, format);
  init_log(log, level, file, line, format, args);
  va_end(args);

#if 0
  log->n += sprintf(log->buff + log->n, " %s:%d ", log->file, log->line);
  vsprintf(log->buff + log->n, log->format, log->args);
  printf("%s\n", log->buff);
#else
  qlog_thread_active();
#endif
}
