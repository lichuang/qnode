/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QMEMPOOL_H__
#define __QMEMPOOL_H__

#include <stddef.h>
#include <stdlib.h>
#include "qcore.h"

#define QALIGN      8
#define QMAX_BYTES  128
#define QFREE_LISTS (QMAX_BYTES / QALIGN)
#define QINIT_BYTES 1024

typedef union qmem_node_t {
  union qmem_node_t   *next;
  char                data[1];
} qmem_node_t;

typedef struct qmem_data_t {
  char                *chunk;
  struct qmem_data_t  *next;
} qmem_data_t;

struct qmem_pool_t {
  qmem_node_t         *free_list[QFREE_LISTS];
  char                *start_free;
  char                *end_free;
  qmem_data_t         *data;
};

qmem_pool_t*  qmem_pool_create();
void          qmem_pool_destroy(qmem_pool_t *pool);

void*         qalloc(qmem_pool_t *pool, size_t size);
void*         qcalloc(qmem_pool_t *pool, size_t size);
void          qfree(qmem_pool_t *pool, void *p, size_t size);

#endif  /* __QMEMPOOL_H__ */
