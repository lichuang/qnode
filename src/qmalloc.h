/*
 * See Copyright Notice in qnode.h
 */
#ifndef __QMALLOC_H__
#define __QMALLOC_H__

#define qalloc_type(T)        (T*)qmalloc(sizeof(T))
#define qalloc_array(T, size) (T*)qmalloc(sizeof(T) * size)

void *qmalloc(size_t size);
void *qrealloc(void *ptr, size_t size);
void qfree(void *ptr);

#endif  /* __QMALLOC_H__ */
