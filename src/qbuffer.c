/*
 * See Copyright Notice in qnode.h
 */

#include "qalloc.h"
#include "qbuffer.h"
#include "qlog.h"

#define QBUFFER_SIZE 1024

int qbuffer_init(qbuffer_t *buffer) {
  buffer->data = qalloc(sizeof(char) * QBUFFER_SIZE);
  if (buffer->data == NULL) {
    return -1;
  }
  buffer->pos  = buffer->len = 0;
  buffer->size = QBUFFER_SIZE;

  return 0;
}

void qbuffer_free(qbuffer_t *buffer) {
  qfree(buffer->data);
}

int qbuffer_extend(qbuffer_t *buffer, uint32_t size) {
  uint32_t      new_size;
  char         *data;

  /* align with 128 bytes */
  new_size = (size + 127) & 0xFF80;
  data = qalloc(sizeof(char) * new_size);
  if (data == NULL) {
    return -1;
  }
  memcpy(data, buffer->data, buffer->size);
  qfree(buffer->data);
  buffer->data = data;
  buffer->size = new_size;

  return 0;
}
