/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qmalloc.h"

static void malloc_default_oom(size_t size) {
  fprintf(stderr, "malloc: Out of memory trying to allocate %zu bytes\n", size);
  fflush(stderr);
  abort();
}

static void (*qmalloc_oom_handler)(size_t) = malloc_default_oom;

void *qmalloc(size_t size) {
  void *ptr = malloc(size);
  if (!ptr) {
    qmalloc_oom_handler(size);
  }
  return ptr;
}

void *qrealloc(void *ptr, size_t size) {
  return realloc(ptr, size);
}

/*
void qfree(void *ptr) {
  free(ptr);
}
*/
