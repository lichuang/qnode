/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qalloc.h"
#include "qassert.h"
#include "qlist.h"
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

static int FREE_LOG_LIST_INIT_NUM = 100;

static int log_level = QLOG_DEBUG;

static void log_init(qlog_t *log, int level, const char* file,
                     long line, const char *format, va_list args);

static qlist_t  free_log_list;
static qmutex_t free_log_list_lock;

const char *
level_str(int level) {
  return log_levels[level];
}

void
qlog_init_free_list() {
  int     i;
  qlog_t *log;

  qmutex_init(&free_log_list_lock);
  qlist_entry_init(&free_log_list);
  qassert(free_log_list.next == &free_log_list);

  for (i = 0; i < FREE_LOG_LIST_INIT_NUM; ++i) {
    log = qcalloc(sizeof(qlog_t));
    if (log == NULL) {
      return;
    }
    qlist_add_tail(&(log->entry), &free_log_list);
  }
}

void
qlog_destroy_free_list() {
  qlog_t  *log;
  qlist_t *pos;

  qmutex_destroy(&free_log_list_lock);
  for (pos = free_log_list.next; pos != &free_log_list; ) {
    log = qlist_entry(pos, qlog_t, entry);
    pos = pos->next;
    qlist_del(&(log->entry));
    qfree(log);
  }
}

void
qlog_free(qlist_t *free_list) {
  qmutex_lock(&free_log_list_lock);
  qlist_add_tail(free_list, &free_log_list);
  qmutex_unlock(&free_log_list_lock);
}

static qlog_t*
qlog_new() {
  qlog_t *log;

  qmutex_lock(&free_log_list_lock);
  if (!qlist_empty(&free_log_list)) {
    log = qlist_entry(free_log_list.next, qlog_t, entry);
    qlist_del_init(&(log->entry));
  } else {
    log = qcalloc(sizeof(qlog_t));
  }
  qmutex_unlock(&free_log_list_lock);

  return log;
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
  qlog_t *log = qlog_new();
  if (log == NULL) {
    return;
  }
  
  va_start(args, format);
  log_init(log, level, file, line, format, args);
  va_end(args);

  log->n = sprintf(log->buff, "%s", logger->time_buff);
  log->n += sprintf(log->buff + log->n, " %s:%d] ", log->file, log->line);
  log->n += vsprintf(log->buff + log->n, log->format, args);
  log->buff[log->n++] = '\n';

  qlogger_add(log);
}
