#include <stdio.h>
#include "ldb_util.h"

void ldb_output(const char * format, ... ) {
  va_list arg;

  va_start(arg, format);
  vfprintf(stdout, format, arg);
  va_end(arg);

  fflush(stdout);
}

