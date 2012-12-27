/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QTIMEUTIL_H__
#define __QTIMEUTIL_H__

#include <sys/time.h>
#include "qtype.h"

/* result in ms */
qtime_t qtime_minus(struct timeval *time1, struct timeval *time2);

#endif  /* __QTIMEUTIL_H__ */
