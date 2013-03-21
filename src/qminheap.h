/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QMINHEAP_H__
#define __QMINHEAP_H__

#include "qcore.h"
#include "qidmap.h"

/* 
 * set data min heap index
 */
typedef void (*set_func_t)(void *data, int index);

/* 
 * get data min heap index
 */
typedef int  (*get_func_t)(void *data);

/* 
 * compare data key
 */
typedef int  (*cmp_func_t)(void *data1, void *data2);

typedef struct qminheap_t {
  void**        data;
  unsigned int  size;
  unsigned int  num;
  cmp_func_t    cmp;
  set_func_t    set;
  get_func_t    get;
  qmem_pool_t   *pool;
} qminheap_t;

#define qminheap_empty(heap)  ((heap)->num == 0)

int     qminheap_init(qminheap_t *heap, qmem_pool_t *pool, cmp_func_t cmp, set_func_t set, get_func_t get);
int     qminheap_push(qminheap_t *heap, void *data);
void*   qminheap_pop(qminheap_t *heap);
void*   qminheap_top(qminheap_t *heap);
int     qminheap_erase(qminheap_t *heap, int index);
void    qminheap_destroy(qminheap_t *heap);

#endif  /* __QMINHEAP_H__ */
