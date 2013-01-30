/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QBUFFER_H__
#define __QBUFFER_H__

#include <stdint.h>

typedef struct qbuffer_t {
  char *data;
  uint32_t size;
  uint32_t len;
} qbuffer_t;

#define qbuffer_reserve(buffer, size) do {    \
  int left = (buffer)->size - (buffer)->len;  \
  if (left < size) {                          \
    qbuffer_extend((buffer), size);           \
  }                                           \
} while(0)

int qbuffer_init(qbuffer_t *buffer);

int qbuffer_extend(qbuffer_t *buffer, uint32_t size);

#endif  /* __QBUFFER_H__ */
