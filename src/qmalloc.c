/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include <stdlib.h>
#include "qmalloc.h"

static void malloc_default_oom(size_t size) {
  fprintf(stderr, "malloc: Out of memory trying to allocate %zu bytes\n", size);
  fflush(stderr);
  abort();
}

static void (*qnode_malloc_oom_handler)(size_t) = malloc_default_oom;

void *qnode_malloc(size_t size) {
  void *ptr = malloc(size);
  if (!ptr) {
    qnode_malloc_oom_handler(size);
  }
  return ptr;
}

void *qnode_calloc(size_t size) {
}

void *qnode_realloc(void *ptr, size_t size) {
}

void qnode_free(void *ptr) {
  free(ptr);
}
