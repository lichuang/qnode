/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QPIPE_H__
#define __QPIPE_H__

#include "qtype.h"

struct queue_t;

typedef struct qnode_pipe_t{
  struct queue_t *queue;
  qnode_ptr_t* w;
  qnode_ptr_t* r;
  qnode_ptr_t* f;
  qnode_ptr_t* c;
} qnode_pipe_t;

qnode_pipe_t* qnode_pipe_new(int size);
void          qnode_pipe_destroy(qnode_pipe_t* pipe);
void          qnode_pipe_write(qnode_pipe_t *pipe, qnode_ptr_t value, int incomplete);
int           qnode_pipe_unwrite(qnode_pipe_t *pipe, qnode_ptr_t value);
int           qnode_pipe_flush(qnode_pipe_t *pipe);
int           qnode_pipe_checkread(qnode_pipe_t *pipe);
int           qnode_pipe_read(qnode_pipe_t *pipe, qnode_ptr_t value);

#endif  /* __QPIPE_H__ */
