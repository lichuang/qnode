/*
 * See Copyright Notice in qnode.h
 */

#include "qalloc.h"
#include "qpalloc.h"

qpool_t *qpool_create(size_t size) {
  qpool_t  *p;

  p = qalloc(size);
  if (p == NULL) {
    return NULL;
  }

  p->d.last = (char *) p + sizeof(qpool_t);
  p->d.end  = (char *) p + size;
  p->d.next = NULL;
  p->d.failed = 0;

  size = size - sizeof(qpool_t);
  //p->max = (size < NGX_MAX_ALLOC_FROM_POOL) ? size : NGX_MAX_ALLOC_FROM_POOL;

  p->current = p;
  p->large = NULL;

  return p;
}

void qpool_destroy(qpool_t *pool) {
  qpool_t          *p, *n;
  qpool_large_t    *l;

  for (l = pool->large; l; l = l->next) {
    if (l->data) {
      qfree(l->data);
    }
  }

  for (p = pool, n = pool->d.next; ;p = n, n = n->d.next) {
    qfree(p);
    if (n == NULL) {
      break;
    }
  }
}

void qpool_reset(qpool_t *pool) {
  qpool_t        *p;
  qpool_large_t  *l;

  for (l = pool->large; l; l = l->next) {
    if (l->data) {
      qfree(l->data);
    }
  }

  pool->large = NULL;

  for (p = pool; p; p = p->d.next) {
    p->d.last = (char *)p + sizeof(qpool_t);
  }
}
