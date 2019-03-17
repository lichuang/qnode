/*
 * Copyright (C) codedump
 */
#ifndef __QNODE_CORE_LOG_H__
#define __QNODE_CORE_LOG_H__

#include <stdarg.h>
#include <stdio.h>

// log level
enum {
  LOG_CRIT  = 0,
  LOG_ERR   = 1,
  LOG_WARN  = 2,
  LOG_INFO  = 3,
  LOG_DEBUG = 4
};

extern int gLogLevel;

extern void Log(int level, const char* file, int line, const char *format, ...);
extern void SetLogLevel(int level);

#define Errorf(args...) if (gLogLevel >= LOG_ERR)   Log(LOG_ERR, __FILE__, __LINE__, args)
#define Debug(args...)  if (gLogLevel >= LOG_DEBUG) Log(LOG_ERR, __FILE__, __LINE__, args)

#endif  // __QNODE_CORE_LOG_H__
