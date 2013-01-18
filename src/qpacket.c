/*
 * See Copyright Notice in qnode.h
 */

#include "qmalloc.h"
#include "qpacket.h"

static qbuffer_t* buffer_create() {
  qbuffer_t *buffer = qalloc_type(qbuffer_t);
  buffer->pos = buffer->size = 0;
  qlist_entry_init(&(buffer->entry));
  return buffer;
}

void qpacket_init(qpacket_t *packet) {
  qlist_entry_init(&(packet->list));
  qbuffer_t *buffer = buffer_create();
  qlist_add(&(buffer->entry), &(packet->list));
  packet->head = packet->current = &(buffer->entry);
  packet->size = 0;
}
