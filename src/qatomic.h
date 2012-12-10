/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QATOMIC_H__
#define __QATOMIC_H__

#include <stdint.h>

void  qatomic_ptr_set(void *ptr, void *val);
void* qatomic_ptr_xchg(void *ptr, void *val);
void* qatomic_ptr_cas(void *ptr, void *cmp, void *val);

typedef uint64_t qatomic_t;

#define qatomic_cas(old, cmp, val) \
  __sync_val_compare_and_swap(old, cmp, val)

#endif  /* __QATOMIC_H__ */
