/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QALLOC_H__
#define __QALLOC_H__

void* qalloc(size_t size);
void* qcalloc(size_t size);

#define qfree free

#endif  /* __QALLOC_H__ */
