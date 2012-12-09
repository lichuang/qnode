/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QSERVER_H__
#define __QSERVER_H__

struct qnode_config_t;
struct qnode_engine_t;
struct qnode_event_t;
struct qnode_thread_t;
struct qnode_mailbox_t;

typedef struct qnode_server_t {
  int thread_num;
  struct qnode_engine_t *engine;
  struct qnode_thread_t  **threads;
  struct qnode_mailbox_t **boxs;
  struct qnode_mailbox_t **thread_boxs;
} qnode_server_t;

qnode_server_t *qnode_server_create(struct qnode_config_t *config);
struct qnode_mailbox_t *qnode_server_get_box(qnode_server_t *server, int tid);

#endif  /* __QSERVER_H__ */
