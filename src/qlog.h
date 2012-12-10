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

#define _LOG(level, fmt... )                                                    \
    if(__log_level >= level) {   struct qlog_t log = {level, __FILE__, __LINE__};    \
        __log(&log, fmt);    \
    } 

#define qerror(args...) _LOG(QLOG_ERR, args)
#define qinfo(args...)  _LOG(QLOG_INFO, args)
#define qdebug(args...) _LOG(QLOG_DEBUG, args)

#endif  /* __QLOG_H__ */
