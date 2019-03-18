/*
 * Copyright (C) codedump
 */
#ifndef __QNODE_BASE_TIME_H__
#define __QNODE_BASE_TIME_H__

#include <stdint.h>
#include <time.h>
#include <sys/time.h>

struct timespec *MakeTimespec(int64_t ms, struct timespec *ts);
uint64_t NowMs();

#endif  // __QNODE_BASE_TIME_H__
