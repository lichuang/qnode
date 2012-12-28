/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QLOG_H__
#define __QLOG_H__

#include <stdarg.h>

#define QLOG_STDERR            0
#define QLOG_EMERG             1
#define QLOG_ALERT             2
#define QLOG_CRIT              3
#define QLOG_ERR               4
#define QLOG_WARN              5
#define QLOG_NOTICE            6
#define QLOG_INFO              7
#define QLOG_DEBUG             8

typedef struct qlog_t {
    int level;
    const char* file;
    int line;
} qlog_t;

extern int __log_level;
extern void __log(struct qlog_t* log, const char*, ...);
extern void qlog(int level, const char* file, long line, const char *format, ...);

#define qerror(args...) qlog(QLOG_ERR, __FILE__, __LINE__, args)
#define qinfo(args...) qlog(QLOG_INFO, __FILE__, __LINE__, args)
#define qdebug(args...) qlog(QLOG_DEBUG, __FILE__, __LINE__, args)

#endif  /* __QLOG_H__ */
