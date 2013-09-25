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

static const char* log_levels[] = {
  "",
  "emerg",
  "alert",
  "crit",
  "error",
  "warn",
  "notice",
  "info",
  "debug",
  NULL
};

static int FREE_LOG_LIST_NUM = 1;

int log_level = QLOG_DEBUG;

static qfreelist_t free_log_list;
static qmutex_t    free_log_list_lock;

const char *
level_str(int level) {
  return log_levels[level];
}

void
qlog_set_level(const char* level) {
  int i;

  for (i = 0; log_levels[i]; ++i) {
    if (!strcmp(level, log_levels[i])) {
      log_level = i;
      return;
    }
  }
}

void
qlog_init_free_list() {
  qfreelist_conf_t conf = QFREELIST_CONF("log free list",
                                         sizeof(qlog_t),
                                         FREE_LOG_LIST_NUM,
                                         NULL, NULL, NULL);
  qmutex_init(&free_log_list_lock);
  qfreelist_init(&free_log_list, &conf);
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

qlog_t*
qlog_new() {
  /* seems freelist no effect, so alloc directly */
  return qcalloc(sizeof(qlog_t));

  qlog_t *log;

  qmutex_lock(&free_log_list_lock);
  log = (qlog_t*)qfreelist_new(&free_log_list);
  qmutex_unlock(&free_log_list_lock);
  if (log != NULL) {
    /* del it from freelist alloc list */
    qlist_del_init(&log->fentry);
  }

  return log;
}

void
qlog(int level, const char* file, int line, const char *format, ...) {
  va_list args;
  int     n;

  if (logger == NULL) {
    return;
  }

  qlog_t *log = qlog_new();
  if (log == NULL) {
    return;
  }
  
  va_start(args, format);
  va_end(args);

  n          = 0;
  n  = snprintf(log->buff, QMAX_LOG_SIZE,
                     "%s", logger->time_buff);
  n += snprintf(log->buff + n, QMAX_LOG_SIZE - n,
                     " %s:%d ", file, line);
  n += vsnprintf(log->buff + n, QMAX_LOG_SIZE - n,
                      format, args);
  log->buff[n++] = '\n';
  log->buff[n++] = '\0';
  log->size = n;
  log->level = level;

  qlogger_add(log);
}
