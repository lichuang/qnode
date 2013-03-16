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

static void* palloc_block(qpool_t *pool, size_t size) {
  char    *m;
  size_t  psize;
  qpool_t *p, *new, *current;

  psize = (size_t)(pool->d.end - (char *)pool);

  m = qalloc(psize);
  if (m == NULL) {
    return NULL;
  }

  new = (qpool_t *) m;

  new->d.end = m + psize;
  new->d.next = NULL;
  new->d.failed = 0;

  m += sizeof(qpool_data_t);
  //m = qalign_ptr(m, QALIGNMENT);
  new->d.last = m + size;

  current = pool->current;

  for (p = current; p->d.next; p = p->d.next) {
    if (p->d.failed++ > 4) {
      current = p->d.next;
    }
  }

  p->d.next = new;

  pool->current = current ? current : new;

  return m;
}

static void* palloc_large(qpool_t *pool, size_t size) {
  void              *p;
  unsigned int       n;
  qpool_large_t  *large;

  p = qalloc(size);
  if (p == NULL) {
    return NULL;
  }

  n = 0;

  for (large = pool->large; large; large = large->next) {
    if (large->data == NULL) {
      large->data = p;
      return p;
    }

    if (n++ > 3) {
      break;
    }
  }

  large = qpalloc(pool, sizeof(qpool_large_t));
  if (large == NULL) {
    qfree(p);
    return NULL;
  }

  large->data = p;
  large->next = pool->large;
  pool->large = large;

  return p;
}
void *qpalloc(qpool_t *pool, size_t size) {
  char      *m;
  qpool_t   *p;

  if (size <= pool->max) {

    p = pool->current;

    do {
      m = p->d.last;

      if ((size_t) (p->d.end - m) >= size) {
        p->d.last = m + size;

        return m;
      }

      p = p->d.next;

    } while (p);

    return palloc_block(pool, size);
  }

  return palloc_large(pool, size);
}

int qpfree(qpool_t *pool, void *p) {
  qpool_large_t  *l;

  for (l = pool->large; l; l = l->next) {
    if (p == l->data) {
      /*
      ngx_log_debug1(NGX_LOG_DEBUG_ALLOC, pool->log, 0,
        "free: %p", l->alloc);
      */
      qfree(l->data);
      l->data = NULL;

      return 0;
    }
  }

  return -1;
}
