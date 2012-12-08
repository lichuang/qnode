/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QATOMIC_H__
#define __QATOMIC_H__

void  qnode_atomic_ptr_set(void *ptr, void *val);
void* qnode_atomic_ptr_xchg(void *ptr, void *val);
void* qnode_atomic_ptr_cas(void *ptr,  void *cmp, void *val);

#endif  /* __QATOMIC_H__ */
