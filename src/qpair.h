/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QPAIR_H__
#define __QPAIR_H__

#include "qengine.h"

struct qnode_msg_t;

/* for communication between main thread and other threads */
struct qnode_pair_t {
    void *reader;
    void *writer;
    int fds[2];
} qnode_pair_t;

qnode_pair_t* qnode_pair_new(void *reader, void *writer,
                             qnode_event_func_t *read, qnode_event_func_t *write);

int qnode_pair_send(struct qnode_msg_t *msg);

#endif  /* __QPAIR_H__ */
