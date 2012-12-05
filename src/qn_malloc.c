/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include <stdlib.h>
#include "qn_malloc.h"

static void malloc_default_oom(size_t size) {
  fprintf(stderr, "qn_malloc: Out of memory trying to allocate %zu bytes\n", size);
  fflush(stderr);
  abort();
}

static void (*qn_malloc_oom_handler)(size_t) = malloc_default_oom;

void *qn_malloc(size_t size) {
  void *ptr = malloc(size);
  if (!ptr) {
    qn_malloc_oom_handler(size);
  }
  return ptr;
}

void *qn_calloc(size_t size) {
}

void *qn_realloc(void *ptr, size_t size) {
}

void qn_free(void *ptr) {
  free(ptr);
}
