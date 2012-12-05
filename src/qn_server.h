/*
 * See Copyright Notice in qnode.h
 */
#ifndef __QN_SERVER_H__
#define __QN_SERVER_H__

struct qn_config_t;
struct qn_dispatcher_t;
struct qn_io_event_t;

typedef struct qn_server_t {
  int thread_num;
  struct qn_dispatcher_t *dispatcher;
  struct qn_io_event_t *event;
} qn_server_t;

qn_server_t *qn_server_create(struct qn_config_t *config);

#endif  /* __QN_SERVER_H__ */
