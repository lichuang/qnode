/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QPIPE_H__
#define __QPIPE_H__

#include "qtype.h"

struct queue_t;

typedef struct qpipe_t{
  struct queue_t *queue;
  qptr_t* w;
  qptr_t* r;
  qptr_t* f;
  qptr_t* c;
} qpipe_t;

qpipe_t* qpipe_new(int size);
void          qpipe_destroy(qpipe_t* pipe);
void          qpipe_write(qpipe_t *pipe, qptr_t value, int incomplete);
int           qpipe_unwrite(qpipe_t *pipe, qptr_t value);
int           qpipe_flush(qpipe_t *pipe);
int           qpipe_checkread(qpipe_t *pipe);
int           qpipe_read(qpipe_t *pipe, qptr_t value);

#endif  /* __QPIPE_H__ */
