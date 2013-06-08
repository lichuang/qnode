/*
 * See Copyright Notice in qnode.h
 */

#include <stdlib.h>
#include <string.h>

void*
qalloc(size_t size) {
  return malloc(size);
}

void*
qcalloc(size_t size) {
  void *p;

  p = qalloc(size);
  if (p) {
    memset(p, 0, size);
  }

  return p;
}

void*
qrealloc(void *p, size_t size) {
  return realloc(p, size);
}

void
qfree(void *p) {
  free(p);
}
