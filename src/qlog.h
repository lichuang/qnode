/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QLOG_H__
#define __QLOG_H__

#include <stdarg.h>
#include <string.h>
#include "qlist.h"

#define QLOG_STDERR            0
#define QLOG_EMERG             1
#define QLOG_ALERT             2
#define QLOG_CRIT              3
#define QLOG_ERR               4
#define QLOG_WARN              5
#define QLOG_NOTICE            6
#define QLOG_INFO              7
#define QLOG_DEBUG             8

#define QMAX_LOG_SIZE    1000
#define QMAX_FORMAT_SIZE 100
#define QMAX_FILE_SIZE 100

typedef struct qlog_t {
  qlist_t list;
  int level;
  char file[QMAX_FILE_SIZE];
  size_t file_len;
  int line;
  char buff[QMAX_LOG_SIZE];
  int n;
  char format[QMAX_FORMAT_SIZE];
  size_t fmt_len;
  va_list args;
} qlog_t;

extern int g_log_level;
extern void qlog(int level, const char* file, long line, const char *format, ...);

#define qerror(format, args...) qlog(QLOG_ERR, __FILE__, __LINE__, format, #args)
#define qinfo(format, args...) qlog(QLOG_ERR, __FILE__, __LINE__, format, #args)
#define qdebug(format, args...) qlog(QLOG_ERR, __FILE__, __LINE__, format, #args)

#endif  /* __QLOG_H__ */
