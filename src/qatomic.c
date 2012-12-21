/*
 * See Copyright Notice in qnode.h
 */

#include "qatomic.h"

void  qatomic_ptr_set(void *ptr, void *val) {
  ptr = val;
}

void* qatomic_ptr_xchg(void *ptr, void *val) {
  void *old;
  void **p = (void**)ptr;
  __asm__ volatile (
      "lock; xchg %0, %2"
      : "=r" (old), "=m" (*p)
      : "m" (*p), "0" (val));
  return old;
}

void* qatomic_ptr_cas(void *ptr, void *cmp, void *val) {
  void *old;
  __asm__ volatile (
      "lock; cmpxchg %2, %3"
      : "=a" (old), "=m" (ptr)
      : "r" (val), "m" (ptr), "0" (cmp)
      : "cc");
  return old;
}
