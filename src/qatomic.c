/*
 * See Copyright Notice in qnode.h
 */

#include "qatomic.h"

void  qnode_atomic_ptr_set(void *ptr, void *val) {
  ptr = val;
}

void* qnode_atomic_ptr_xchg(void *ptr, void *val) {
  void *old;
  __asm__ volatile (
      "lock; xchg %0, %2"
      : "=r" (old), "=m" (ptr)
      : "m" (ptr), "0" (val));
  return old;
}

void* qnode_atomic_ptr_cas(void *ptr, void *cmp, void *val) {
  void *old;
  __asm__ volatile (
      "lock; cmpxchg %2, %3"
      : "=a" (old), "=m" (ptr)
      : "r" (val), "m" (ptr), "0" (cmp)
      : "cc");
  return old;
}

