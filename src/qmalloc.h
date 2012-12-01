/*
 * See Copyright Notice in qnode.h
 */
#ifndef __QMALLOC_H__
#define __QMALLOC_H__

#define qnode_alloc_type(T)        (T*)qnode_malloc(sizeof(T))
#define qnode_alloc_array(T, size) (T*)qnode_malloc(sizeof(T) * size)

void *qnode_malloc(size_t size);
void *qnode_calloc(size_t size);
void *qnode_realloc(void *ptr, size_t size);
void qnode_free(void *ptr);

#endif  /* __QMALLOC_H__ */
