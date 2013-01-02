/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qassert.h"
#include "qatomic.h"
#include "qmalloc.h"
#include "qpipe.h"

typedef struct chunk_t {
  qptr_t* values;
  struct chunk_t *prev;
  struct chunk_t *next;
} chunk_t;

typedef struct queue_t {
  chunk_t *begin_chunk;
  int begin_pos;
  chunk_t *back_chunk;
  int back_pos;
  chunk_t *end_chunk;
  int end_pos;
  chunk_t *spare_chunk;
  int size;
} queue_t;

static queue_t* queue_new(int size) {
  queue_t *queue = qalloc_type(queue_t);
  qalloc_assert(queue);
  queue->begin_chunk = qalloc_type(chunk_t);
  qalloc_assert(queue->begin_chunk);
  queue->begin_chunk->values = (qptr_t*)qmalloc(size * sizeof(qptr_t));
  qalloc_assert(queue->begin_chunk->values);
  queue->begin_pos = 0;
  queue->back_chunk = NULL;
  queue->back_pos = 0;
  queue->end_chunk = queue->begin_chunk;
  queue->end_pos = 0;
  queue->spare_chunk = NULL;
  queue->size = size;
  return queue;
}

static void queue_destory(queue_t *queue) {
  while (1) {
    if (queue->begin_chunk == queue->end_chunk) {
      qfree (queue->begin_chunk);
      break;
    } 
    chunk_t *o = queue->begin_chunk;
    queue->begin_chunk = queue->begin_chunk->next;
    qfree (o);
  }

  chunk_t *sc = (chunk_t*)qatomic_ptr_xchg(queue->spare_chunk, NULL);
  if (sc) {
    qfree (sc);
  }
  qfree(queue);
}

static qptr_t* queue_front(queue_t *queue) {
  return &(queue->begin_chunk->values[queue->begin_pos]);
}

static qptr_t* queue_back(queue_t *queue) {
  return &(queue->back_chunk->values[queue->back_pos]);
}

static void queue_add(queue_t *queue, qptr_t value) {
  queue->back_chunk->values[queue->back_pos] = value;
}

static void queue_push(queue_t* queue) {
  queue->back_chunk = queue->end_chunk;
  queue->back_pos   = queue->end_pos;

  if (++queue->end_pos != queue->size) {
    return;
  }

  chunk_t *sc = qatomic_ptr_xchg(queue->spare_chunk, NULL);
  if (sc) {
    queue->end_chunk->next = sc;
    sc->prev = queue->end_chunk;
  } else {
    queue->end_chunk->next = qalloc_type(chunk_t); 
    qalloc_assert(queue->end_chunk->next);
    queue->end_chunk->next->prev = queue->end_chunk;
  }
  queue->end_chunk = queue->end_chunk->next;
  queue->end_pos = 0;
}

static void queue_unpush(queue_t *queue) {
  if (queue->back_pos) {
    --queue->back_pos;
  } else {
    queue->back_pos = queue->size - 1;
    queue->back_chunk = queue->back_chunk->prev;
  }
  if (queue->end_pos) {
    --queue->end_pos;
  } else {
    queue->end_pos = queue->size - 1;
    queue->end_chunk = queue->end_chunk->prev;
    qfree(queue->end_chunk->next);
    queue->end_chunk->next = NULL;
  }
}

static void queue_pop(queue_t *queue) {
  if (++queue->begin_pos == queue->size) {
    chunk_t *o = queue->begin_chunk;
    queue->begin_chunk = queue->begin_chunk->next;
    queue->begin_chunk->prev = NULL;
    queue->begin_pos = 0;
    chunk_t *cs = qatomic_ptr_xchg(queue->spare_chunk, o);
    if (cs) {
      qfree (cs);
    }
  }
}

qpipe_t* qpipe_new(int size) {
  qpipe_t *pipe = qalloc_type(qpipe_t);
  qalloc_assert(pipe);
  queue_t *queue = queue_new(size);
  queue_push(queue);
  pipe->r = pipe->w = pipe->f = queue_back(queue);
  qatomic_ptr_set(pipe->c, queue_back(queue));
  return pipe;
}

void qpipe_destroy(qpipe_t* pipe) {
  queue_destory(pipe->queue);
  qfree(pipe);
}

void qpipe_write(qpipe_t *pipe, qptr_t value, int incomplete) {
  queue_t *queue = pipe->queue;
  queue_add(queue, value);
  queue_push(queue);

  if (!incomplete) {
    pipe->f = queue_back(queue);
  }
}

int qpipe_unwrite(qpipe_t *pipe, qptr_t value) {
  queue_t *queue = pipe->queue;
  if (pipe->f == queue_back(queue)) {
    return 0;
  }
  queue_unpush(queue);
  value = *(queue_back(queue));
  return 1;
}

int qpipe_flush(qpipe_t *pipe) {
  if (pipe->w == pipe->f) {
    return 1;
  }

  qptr_t *ptr = qatomic_ptr_cas(pipe->c, pipe->w, pipe->f);
  if (ptr != pipe->w) {
    qatomic_ptr_set(pipe->c, pipe->f);
    pipe->w = pipe->f;
    return 0;
  }
  pipe->w = pipe->f;
  return 1;
}

int qpipe_checkread(qpipe_t *pipe) {
  queue_t *queue = pipe->queue;
  if (queue_front(queue) != pipe->r && pipe->r) {
    return 1;
  }
  pipe->r = qatomic_ptr_cas(pipe->c, queue_front(queue), NULL);
  if (queue_front(queue) == pipe->r || !pipe->r) {
    return 0;
  }
  return 1;
}

int qpipe_read(qpipe_t *pipe, qptr_t value) {
  queue_t *queue = pipe->queue;
  if (!qpipe_checkread(pipe)) {
    return 0;
  }
  value = *(queue_front(queue));
  queue_pop(queue);
  return 1;
}
