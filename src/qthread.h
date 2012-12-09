/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QTHREAD_H__
#define __QTHREAD_H__

struct qnode_engine_t;
struct qnode_mailbox_t;
struct qnode_server_t;

typedef struct qnode_thread_t {
    int thread_id;
    struct qnode_engine_t *engine;
    struct qnode_mailbox_t *box;        /* server send to thread */
    struct qnode_mailbox_t *server_box; /* thread send to server */
} qnode_thread_t;

qnode_thread_t* qnode_thread_new(struct qnode_server_t *server, int tid);
void qnode_thread_destroy(qnode_thread_t *thread);
struct qnode_mailbox_t qnode_thread_mailbox(qnode_thread_t *thread);

#endif  /* __QTHREAD_H__ */
