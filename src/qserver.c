/*
 * See Copyright Notice in qnode.h
 */

#include "qassert.h"
#include "qconfig.h"
#include "qevent.h"
#include "qmalloc.h"
#include "qnet.h"
#include "qserver.h"

static int init_server_event(struct qn_server_t *server) {
  int fd = qn_net_create_listener(9999, "127.0.0.1");
  if (fd < 0) {
    return -1;
  }
}

qnode_server_t *qnode_server_create(struct qnode_config_t *config) {
  qnode_assert(config);
  qnode_assert(config->thread_num > 0);
  qnode_server_t *server = qnode_alloc_type(qn_server_t);
  server->thread_num = config->thread_num;
  server->engine = qnode_alloc_type(qn_engine_t);
  server->event = qnode_alloc_type(qn_io_event_t);
  if (init_server_event(server) < 0) {
    qnode_free(server->event);
    qnode_free(server->engine);
    qnode_free(server);
    return NULL;
  }

  return server;
}

