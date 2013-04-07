/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QALLOC_H__
#define __QALLOC_H__

#include <stddef.h>

void* qalloc(size_t size);
void* qcalloc(size_t size);
void* qrealloc(void *p, size_t size);
void  qfree(void *p);

#endif  /* __QALLOC_H__ */
