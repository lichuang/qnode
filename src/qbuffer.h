/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QBUFFER_H__
#define __QBUFFER_H__

#include <stdint.h>
#include "qcore.h"

typedef struct qbuffer_t {
  char *data;
  int   size;
  int   start;
  int   end;
  int   len;
} qbuffer_t;

#define qbuffer_reserve(buffer, len)  \
  if ((buffer)->size < (len)) {       \
    qbuffer_extend((buffer), (len));  \
  } 

int     qbuffer_init(qbuffer_t *buffer);
void    qbuffer_free(qbuffer_t *buffer);
int     qbuffer_extend(qbuffer_t *buffer, uint32_t size);
char*   qbuffer_read(qbuffer_t *buffer, int size);
void    qbuffer_write(qbuffer_t *buffer, const char *data, int size);

#endif  /* __QBUFFER_H__ */
