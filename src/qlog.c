/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qlog.h"

void _log(struct qnode_log_t* log, ...) {
    va_start(log->ap, log);
    char buf[256] = {0};
    vfprintf(buf, log->fmt, log->ap);
    printf("%s", buf);
    va_end(log->ap);
}
