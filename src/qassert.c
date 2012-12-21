/*
 * See Copyright Notice in qnode.h
 */

#include "qassert.h"
#include "qlog.h"

#ifdef DEBUG

void qassert_error(char* expr, char* file, int line) {
  qerror("ASSERT [%s:%d] %s error", file, line, expr);
}

#endif
