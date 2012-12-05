/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QLOG_H__
#define __QLOG_H__

#include <stdarg.h>

enum qnode_log_priority_t {
    QNODE_LOG_NONE      = 0,
    QNODE_LOG_TRACE     = 1,
    QNODE_LOG_DEBUF     = 2,
    QNODE_LOG_INFO      = 3,
    QNODE_LOG_NOTICE    = 4,
    QNODE_LOG_WARNING   = 5,
    QNODE_LOG_ERROR     = 6,
    QNODE_LOG_CRITICAL  = 7,
    QNODE_LOG_ALERT     = 8,
    QNODE_LOG_EMERGENCY = 9,
};

typedef struct qnode_log_t {
    //struct LogCategory* cat;
    int priority;
    char* file;
    char* function;
    int line;
    char *fmt;
    va_list ap;
} qnode_log_t;

extern void _log(struct qnode_log_t* log, ...);

#define _LOG_ISENABLEDV(priority) (1)

#define _LOG_PRE(priority, fmt) do {                                                \
    if (_LOG_ISENABLEDV(priority)) {                                                \
        struct qnode_log_t _log = {priority,__FILE__,__FUNCTION__,__LINE__, fmt};   \
        _log_logEvent(&_log_ev

#define _LOG_POST                                                                   \
            );                                                                      \
    } } while(0)

#define QNODE_INFO(fmt, msg) _LOG_PRE(QNODE_LOG_INFO, fmt) ,msg _LOG_POST

#endif  /* __QLOG_H__ */
