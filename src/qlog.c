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

static int FREE_LOG_LIST_INIT_NUM = 10;

static int log_level = QLOG_DEBUG;

static void log_init(qlog_t *log, int level, const char* file,
                     long line, const char *format, va_list args);

static qfreelist_t free_log_list;
static qmutex_t    free_log_list_lock;

const char *
level_str(int level) {
  return log_levels[level];
}

void
qlog_init_free_list() {
  qmutex_init(&free_log_list_lock);
  qfreelist_init(&free_log_list, "log free list",
                 sizeof(qlog_t), FREE_LOG_LIST_INIT_NUM);
}

void
qlog_destroy_free_list() {
  qmutex_lock(&free_log_list_lock);
  qfreelist_destroy(&free_log_list);
  qmutex_unlock(&free_log_list_lock);
  qmutex_destroy(&free_log_list_lock);
}

void
qlog_free(qlist_t *free_list) {
  qmutex_lock(&free_log_list_lock);
  qlist_splice_tail(free_list, &(free_log_list.free));
  qmutex_unlock(&free_log_list_lock);
}

static qlog_t*
qlog_new() {
  qlog_t *log;

  qmutex_lock(&free_log_list_lock);
  log = (qlog_t*)qfreelist_alloc(&free_log_list);
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
  log->buff[log->n++] = '\0';

  qlogger_add(log);
}

void
qlog_freelist_print() {
  /*
  qlog_t  *log;
  qlist_t *pos;

  for (pos = free_log_list.next; pos != &free_log_list; ) {
    log = qlist_entry(pos, qlog_t, free);
    pos = pos->next;
    printf("free log: %p\n", log);
  }
  */
}
