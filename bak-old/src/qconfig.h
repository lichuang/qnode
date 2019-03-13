/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QCONFIG_H__
#define __QCONFIG_H__

#include "qcore.h"
#include "qstring.h"

struct qconfig_t {
  int           worker;
  int           daemon;
  int           log_size;
  int           recycle_internal;
  qstring_t     script_path;
  qstring_t     main;
  qstring_t     log_path;
  qstring_t     log_level;
  char          cwd[1024];
};

int  qconfig_init(const char *filename);
void qconfig_free();

extern qconfig_t config;

#endif  /* __QCONFIG_H__ */
