/*
 * See Copyright Notice in qnode.h
 */

#include <stdlib.h>
#include "qassert.h"
#include "qlog.h"

#ifdef DEBUG

void qassert_error(char* expr, char* file, int line) {
  abort();
  qerror("ASSERT [%s:%d] %s error", file, line, expr);
}

#endif
