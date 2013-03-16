/*
 * See Copyright Notice in qnode.h
 */

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "qalloc.h"

void* qalloc(size_t size) {
  void* p;

  p = malloc(size);
  if (p == NULL) {
    return p;
    /*
    ngx_log_error(NGX_LOG_EMERG, log, ngx_errno,
      "malloc(%uz) failed", size);
      */
  }

  //ngx_log_debug2(NGX_LOG_DEBUG_ALLOC, log, 0, "malloc: %p:%uz", p, size);

  return p;
}


void* qcalloc(size_t size) {
  void* p;

  p = qalloc(size);

  if (p) {
    memset(p, 0, size);
  }

  return p;
}
