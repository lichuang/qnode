/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QBUFFER_H__
#define __QBUFFER_H__

#include <stdint.h>
#include "qcore.h"

typedef struct qbuffer_t {
  char        *data;
  uint32_t    size;
  uint32_t    pos;
  uint32_t    len;
  qmem_pool_t *pool;
} qbuffer_t;

#define qbuffer_reserve(buffer, len)  \
  if ((buffer)->size < (len)) {       \
    qbuffer_extend((buffer), (len));  \
  } 

int  qbuffer_init(qmem_pool_t *pool, qbuffer_t *buffer);
void qbuffer_free(qbuffer_t *buffer);
int  qbuffer_extend(qbuffer_t *buffer, uint32_t size);

#endif  /* __QBUFFER_H__ */
