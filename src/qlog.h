/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QLOG_H__
#define __QLOG_H__

#include <stdarg.h>

#define QNODE_LOG_STDERR            0
#define QNODE_LOG_EMERG             1
#define QNODE_LOG_ALERT             2
#define QNODE_LOG_CRIT              3
#define QNODE_LOG_ERR               4
#define QNODE_LOG_WARN              5
#define QNODE_LOG_NOTICE            6
#define QNODE_LOG_INFO              7
#define QNODE_LOG_DEBUG             8

typedef struct qnode_log_t {
    int level;
    const char* file;
    int line;
} qnode_log_t;

extern int __log_level;
extern void __log(struct qnode_log_t* log, const char*, ...);

#define _LOG(level, fmt... )                                                    \
    if(__log_level >= level) {   struct qnode_log_t log = {level, __FILE__, __LINE__};    \
        __log(&log, fmt);    \
    } 

#define qnode_error(args...) _LOG(QNODE_LOG_ERR, args)
#define qnode_info(args...)  _LOG(QNODE_LOG_INFO, args)
#define qnode_debug(args...) _LOG(QNODE_LOG_DEBUG, args)

#endif  /* __QLOG_H__ */
