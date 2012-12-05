/*
 * See Copyright Notice in qnode.h
 */
#ifndef __QN_MALLOC_H__
#define __QN_MALLOC_H__

#define qn_alloc_type(T)        (T*)qn_malloc(sizeof(T))
#define qn_alloc_array(T, size) (T*)qn_malloc(sizeof(T) * size)

void *qn_malloc(size_t size);
void *qn_calloc(size_t size);
void *qn_realloc(void *ptr, size_t size);
void qn_free(void *ptr);

#endif  /* __QN_MALLOC_H__ */
