/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qassert.h"
#include "qatomic.h"
#include "qmalloc.h"
#include "qpipe.h"

typedef struct chunk_t {
  qnode_ptr_t* values;
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
  queue_t *queue = qnode_alloc_type(queue_t);
  qnode_alloc_assert(queue);
  queue->begin_chunk = qnode_alloc_type(chunk_t);
  qnode_alloc_assert(queue->begin_chunk);
  queue->begin_chunk->values = (qnode_ptr_t*)qnode_malloc(size * sizeof(qnode_ptr_t));
  qnode_alloc_assert(queue->begin_chunk->values);
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
      qnode_free (queue->begin_chunk);
      break;
    } 
    chunk_t *o = queue->begin_chunk;
    queue->begin_chunk = queue->begin_chunk->next;
    qnode_free (o);
  }

  chunk_t *sc = (chunk_t*)qnode_atomic_ptr_xchg(queue->spare_chunk, NULL);
  if (sc) {
    qnode_free (sc);
  }
  qnode_free(queue);
}

static qnode_ptr_t* queue_front(queue_t *queue) {
  return &(queue->begin_chunk->values[queue->begin_pos]);
}

static qnode_ptr_t* queue_back(queue_t *queue) {
  return &(queue->back_chunk->values[queue->back_pos]);
}

static void queue_add(queue_t *queue, qnode_ptr_t value) {
  queue->back_chunk->values[queue->back_pos] = value;
}

static void queue_push(queue_t* queue) {
  queue->back_chunk = queue->end_chunk;
  queue->back_pos   = queue->end_pos;

  if (++queue->end_pos != queue->size) {
    return;
  }

  chunk_t *sc = qnode_atomic_ptr_xchg(queue->spare_chunk, NULL);
  if (sc) {
    queue->end_chunk->next = sc;
    sc->prev = queue->end_chunk;
  } else {
    queue->end_chunk->next = qnode_alloc_type(chunk_t); 
    qnode_alloc_assert(queue->end_chunk->next);
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
    qnode_free(queue->end_chunk->next);
    queue->end_chunk->next = NULL;
  }
}

static void queue_pop(queue_t *queue) {
  if (++queue->begin_pos == queue->size) {
    chunk_t *o = queue->begin_chunk;
    queue->begin_chunk = queue->begin_chunk->next;
    queue->begin_chunk->prev = NULL;
    queue->begin_pos = 0;
    chunk_t *cs = qnode_atomic_ptr_xchg(queue->spare_chunk, o);
    if (cs) {
      qnode_free (cs);
    }
  }
}

qnode_pipe_t* qnode_pipe_new(int size) {
  qnode_pipe_t *pipe = qnode_alloc_type(qnode_pipe_t);
  qnode_alloc_assert(pipe);
  queue_t *queue = queue_new(size);
  queue_push(queue);
  pipe->r = pipe->w = pipe->f = queue_back(queue);
  qnode_atomic_ptr_set(pipe->c, queue_back(queue));
  return pipe;
}

void qnode_pipe_destroy(qnode_pipe_t* pipe) {
  queue_destory(pipe->queue);
  qnode_free(pipe);
}

void qnode_pipe_write(qnode_pipe_t *pipe, qnode_ptr_t value, int incomplete) {
  queue_t *queue = pipe->queue;
  queue_add(queue, value);
  queue_push(queue);

  if (!incomplete) {
    pipe->f = queue_back(queue);
  }
}

int qnode_pipe_unwrite(qnode_pipe_t *pipe, qnode_ptr_t value) {
  queue_t *queue = pipe->queue;
  if (pipe->f == queue_back(queue)) {
    return 0;
  }
  queue_unpush(queue);
  value = *(queue_back(queue));
  return 1;
}

int qnode_pipe_flush(qnode_pipe_t *pipe) {
  if (pipe->w == pipe->f) {
    return 1;
  }

  qnode_ptr_t *ptr = qnode_atomic_ptr_cas(pipe->c, pipe->w, pipe->f);
  if (ptr != pipe->w) {
    qnode_atomic_ptr_set(pipe->c, pipe->f);
    pipe->w = pipe->f;
    return 0;
  }
  pipe->w = pipe->f;
  return 1;
}

int qnode_pipe_checkread(qnode_pipe_t *pipe) {
  queue_t *queue = pipe->queue;
  if (queue_front(queue) != pipe->r && pipe->r) {
    return 1;
  }
  pipe->r = qnode_atomic_ptr_cas(pipe->c, queue_front(queue), NULL);
  if (queue_front(queue) == pipe->r || !pipe->r) {
    return 0;
  }
  return 1;
}

int qnode_pipe_read(qnode_pipe_t *pipe, qnode_ptr_t value) {
  queue_t *queue = pipe->queue;
  if (!qnode_pipe_checkread(pipe)) {
    return 0;
  }
  value = *(queue_front(queue));
  queue_pop(queue);
  return 1;
}
