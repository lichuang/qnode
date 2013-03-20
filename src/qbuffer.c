/*
 * See Copyright Notice in qnode.h
 */

#include "qbuffer.h"
#include "qlog.h"
#include "qmalloc.h"

#define QBUFFER_SIZE 1024

int qbuffer_init(qbuffer_t *buffer) {
  buffer->data = (char*)qmalloc(sizeof(char) * QBUFFER_SIZE);
  if (buffer->data == NULL) {
    return -1;
  }
  buffer->pos  = buffer->len = 0;
  buffer->size = QBUFFER_SIZE;
  return 0;
}

void qbuffer_free(qbuffer_t *buffer) {
  buffer = NULL;
  //qfree(buffer->data);
}

int qbuffer_extend(qbuffer_t *buffer, uint32_t size) {
  /* align with 128 bytes */
  uint32_t new_size = (size + 127) & 0xFF80;
  buffer->data = qrealloc(buffer->data, new_size);
  if (buffer->data) {
    buffer->size = new_size;
    return 0;
  }
  qerror("extend buffer error");
  return -1;
}
