/*
 * See Copyright Notice in qnode.h
 */

#include <string.h>
#include "qmempool.h"

static inline size_t round_up(size_t bytes) {
  return (((bytes + QALIGN - 1) & ~(QALIGN - 1)));
}

static inline size_t free_list_index(size_t bytes) {
  return (((bytes + QALIGN - 1) / QALIGN - 1));
}

static int new_mem_data(qmem_pool_t *pool) {
  qmem_data_t *data;

  data = malloc(sizeof(qmem_data_t));
  if (data == NULL) {
    return -1;
  }
  data->chunk = pool->start_free;
  data->next = pool->data;
  pool->data = data;

  return 0;
}

static char* chunk_alloc(qmem_pool_t *pool, size_t size, int *nobjs) {
  int           i;
  char         *result;
  size_t        total_bytes;
  size_t        bytes_left;
  qmem_node_t **list, *p;

  total_bytes = size * (*nobjs);
  bytes_left  = pool->end_free - pool->start_free;
  if (bytes_left > total_bytes) {
    result = pool->start_free;
    pool->start_free += total_bytes;
    return result;
  }

  if (bytes_left > size) {
    *nobjs = bytes_left / size;
    total_bytes = size * (*nobjs);
    result = pool->start_free;
    pool->start_free += total_bytes;
    return result;
  }

  if (bytes_left > 0) {
    list = pool->free_list + free_list_index(bytes_left);
    ((qmem_node_t*)pool->start_free)->next = *list;
    *list = ((qmem_node_t*)pool->start_free);
  }

  pool->start_free = malloc(QINIT_BYTES);
  if (pool->start_free == NULL) {
    for (i = size; i < QMAX_BYTES; i += QALIGN) {
      list = pool->free_list + free_list_index(i);
      p = *list;
      if (p != NULL) {
        *list = p->next;
        pool->start_free = (char *)p;
        pool->end_free   = pool->start_free + i;
        return chunk_alloc(pool, size, nobjs);
      }
    }
    return NULL;
  }
  if (new_mem_data(pool) < 0) {
    return NULL;
  }
  pool->end_free = pool->start_free + QINIT_BYTES;

  return chunk_alloc(pool, size, nobjs);
}

static void* refill(qmem_pool_t *pool, size_t n) {
  int          i, nobjs;
  char        *chunk;
  qmem_node_t **list;
  qmem_node_t *result;
  qmem_node_t *current, *next;

  nobjs = 10;
  chunk = chunk_alloc(pool, n, &nobjs);

  if (nobjs == 1) {
    return chunk;
  }


  list = pool->free_list + free_list_index(n);
  result = (qmem_node_t *)chunk;
  *list = next = (qmem_node_t*)(chunk + n);
  for (i = 1; ; ++i) {
    current = next;
    next = (qmem_node_t *)((char *)next + n);
    if (nobjs - 1 == i) {
      current->next = NULL;
      break;
    } else {
      current->next = next;
    }
  }

  return (result);
}

qmem_pool_t* qmem_pool_create() {
  qmem_pool_t *pool;

  pool = malloc(sizeof(qmem_pool_t));
  if (pool == NULL) {
    return NULL;
  }

  pool->start_free = malloc(QINIT_BYTES);
  if (pool->start_free == NULL) {
    free(pool);
    return NULL;
  }
  pool->end_free = pool->start_free + QINIT_BYTES;
  if (new_mem_data(pool) < 0) {
    free(pool->start_free);
    free(pool);
    return NULL;
  }
  memset(pool->free_list, 0, sizeof(pool->free_list));
  return pool;
}

void qmem_pool_destroy(qmem_pool_t *pool) {
  qmem_data_t *data, *tmp;

  for (data = pool->data; data; ) {
    tmp = data;
    data = data->next;
    free(tmp->chunk);
    free(tmp);
  }
  free(pool);
}

void* qalloc(qmem_pool_t *pool, size_t size) {
  void        *p;
  qmem_node_t **list, *result;

  if (size > QMAX_BYTES) {
    return malloc(size);
  }

  list = pool->free_list + free_list_index(size);
  result = *list;

  if (result == NULL) {
    p = refill(pool, round_up(size));
    return p;
  }
  *list = result->next;

  return result;
}

void* qcalloc(qmem_pool_t *pool, size_t size) {
  void *result;

  result = qalloc(pool, size);
  if (result) {
    bzero(result, size);
  }

  return result;
}

void qfree(qmem_pool_t *pool, void *p, size_t size) {
  qmem_node_t *node;
  qmem_node_t **list;

  node = (qmem_node_t*)p;
  if (size > QMAX_BYTES) {
    free(p);
    return;
  }

  list = pool->free_list + free_list_index(size);
  node->next = *list;
  *list = node;
}
