/*
 * See Copyright Notice in qnode.h
 */

#include "qtimeutil.h"

qtime_t qtime_minus(struct timeval *time1, struct timeval *time2) {
  qtime_t sec  = time1->tv_sec  - time2->tv_sec;
  qtime_t usec = time1->tv_usec - time2->tv_usec;
  return (sec / 1000 + usec * 1000);
}

