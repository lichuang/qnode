/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QTHREAD_H__
#define __QTHREAD_H__

struct qnode_engine_t;

typedef struct qnode_thread_t {
    struct qnode_engine_t *engine;
} qnode_thread_t;

#endif  /* __QTHREAD_H__ */
