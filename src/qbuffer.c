/*
 * See Copyright Notice in qnode.h
 */

#include "qalloc.h"
#include "qbuffer.h"
#include "qlimits.h"
#include "qlog.h"
#include "qmutex.h"

#define QBUFFER_FREE_NUM 100

static qfreelist_t free_buffer_list;
static qmutex_t    free_buffer_list_lock;

static int  buffer_init(void *data);
static void buffer_destroy(void *data);

int
qbuffer_init_freelist() {
  qmutex_init(&free_buffer_list_lock);
  return qfreelist_init(&free_buffer_list,
                        "buffer free list",
                        sizeof(qbuffer_t),
                        QBUFFER_FREE_NUM,
                        buffer_init, buffer_destroy);
}

qbuffer_t*
qbuffer_new() {
  qbuffer_t *buffer;

  qmutex_lock(&free_buffer_list_lock);
  buffer = (qbuffer_t*)qfreelist_alloc(&free_buffer_list);
  qmutex_unlock(&free_buffer_list_lock);

  return buffer;
}

void
qbuffer_free(qbuffer_t *buffer) {
  qmutex_lock(&free_buffer_list_lock);
  qfreelist_free(&free_buffer_list, (qfree_item_t*)buffer);
  qmutex_unlock(&free_buffer_list_lock);
  if (buffer->size > QBUFFER_SIZE) {
    buffer->data = qrealloc(buffer->data, QBUFFER_SIZE);
    buffer->size = QBUFFER_SIZE;
  }
  qbuffer_reset(buffer);
}

int
qbuffer_extend(qbuffer_t *buffer, uint32_t size) {
  uint32_t      new_size;
  char         *data;

  /* align with 128 bytes */
  new_size = (size + 127) & 0xFF80;
  data = qrealloc(buffer->data, sizeof(char) * new_size);
  if (data == NULL) {
    return QERROR;
  }
  buffer->data = data;
  buffer->size = new_size;

  return QOK;
}

char*
qbuffer_read(qbuffer_t *buffer, int size) {
  char *p;

  p = buffer->data + buffer->start;
  buffer->start += size;

  return p;
}

int
qbuffer_write(qbuffer_t *buffer, const char *data, int size) {
  qbuffer_reserve(buffer, size); 
  if (buffer->data == NULL) {
    return QERROR;
  }
  memcpy(buffer->data + buffer->end, data, size);
  buffer->end += size;

  return QOK;
}

static int
buffer_init(void *data) {
  qbuffer_t *buffer;

  buffer = (qbuffer_t*)data;
  buffer->data = qalloc(sizeof(char) * QBUFFER_SIZE);
  if (buffer->data == NULL) {
    return QERROR;
  }
  buffer->start  = buffer->end = 0;
  buffer->size = QBUFFER_SIZE;

  return QOK;
}

static void
buffer_destroy(void *data) {
  qbuffer_t *buffer;

  buffer = (qbuffer_t*)data;
  qfree(buffer->data);
}

void
qbuffer_reinit(qbuffer_t *buffer) {
  if (buffer->size > QBUFFER_SIZE) {
    buffer->data = qrealloc(buffer->data, QBUFFER_SIZE);
    buffer->size = QBUFFER_SIZE;
  }
  qbuffer_reset(buffer);
}
