/*
 * See Copyright Notice in qnode.h
 */
#ifndef __QSERVER_H__
#define __QSERVER_H__

struct qnode_config_t;
struct qnode_dispatcher_t;
struct qnode_event_t;

typedef struct qnode_server_t {
  int thread_num;
  struct qnode_dispatcher_t *dispatcher;
  struct qnode_event_t *event;
} qnode_server_t;

qnode_server_t *qnode_server_create(struct qnode_config_t *config);

#endif  /* __QSERVER_H__ */
