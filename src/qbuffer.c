/*
 * See Copyright Notice in qnode.h
 */

#include "qbuffer.h"
#include "qlog.h"
#include "qmempool.h"

#define QBUFFER_SIZE 1024

int qbuffer_init(qmem_pool_t *pool, qbuffer_t *buffer) {
  buffer->data = qalloc(pool, sizeof(char) * QBUFFER_SIZE);
  if (buffer->data == NULL) {
    return -1;
  }
  buffer->pos  = buffer->len = 0;
  buffer->size = QBUFFER_SIZE;
  buffer->pool = pool;
  return 0;
}

void qbuffer_free(qbuffer_t *buffer) {
  qfree(buffer->pool, buffer->data, buffer->size);
}

int qbuffer_extend(qbuffer_t *buffer, uint32_t size) {
  uint32_t      new_size;
  char         *data;
  qmem_pool_t  *pool;

  pool = buffer->pool;
  /* align with 128 bytes */
  new_size = (size + 127) & 0xFF80;
  data = qalloc(pool, sizeof(char) * new_size);
  if (data == NULL) {
    return -1;
  }
  memcpy(data, buffer->data, buffer->size);
  qfree(pool, buffer->data, buffer->size);
  buffer->data = data;
  buffer->size = new_size;

  return 0;
}
