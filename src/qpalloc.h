/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QPALLOC_H__
#define __QPALLOC_H__

#include <stddef.h>
#include <stdint.h>

#ifndef QALIGNMENT
#define QALIGNMENT   sizeof(unsigned long)
#endif

#define qalign(d, a)     (((d) + (a - 1)) & ~(a - 1))
#define qalign_ptr(p, a)                                                   \
      (u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))

typedef struct qpool_large_t {
  struct qpool_large_t   *next;
  void            *data;
} qpool_large_t;

struct qpool_pool_t;

typedef struct qpool_data_t {
  char            *last;
  char            *end;
  struct qpool_t  *next;
  unsigned int    failed;
} qpool_data_t;

typedef struct qpool_t {
  qpool_data_t    d;
  size_t          max;
  struct qpool_t  *current;
  qpool_large_t   *large;
} qpool_t;

qpool_t *qpool_create(size_t size);
void qpool_destroy(qpool_t *pool);
void qpool_reset(qpool_t *pool);

void *qpalloc(qpool_t *pool, size_t size);
int qpfree(qpool_t *pool, void *p);
/*
void *ngx_pnalloc(ngx_pool_t *pool, size_t size);
void *ngx_pcalloc(ngx_pool_t *pool, size_t size);
void *ngx_pmemalign(ngx_pool_t *pool, size_t size, size_t alignment);
*/

#endif  /* __QPALLOC_H__ */
