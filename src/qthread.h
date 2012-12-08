/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QTHREAD_H__
#define __QTHREAD_H__

struct qnode_engine_t;
struct qnode_server_t;

typedef struct qnode_thread_t {
    int thread_id;
    struct qnode_engine_t *engine;
} qnode_thread_t;

qnode_thread_t* qnode_thread_new(struct qnode_server_t *server);

#endif  /* __QTHREAD_H__ */
