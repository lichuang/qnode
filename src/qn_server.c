/*
 * See Copyright Notice in qnode.h
 */

#include "qn_assert.h"
#include "qn_config.h"
#include "qn_event.h"
#include "qn_malloc.h"
#include "qn_net.h"
#include "qn_server.h"

static int init_server_event(struct qn_server_t *server) {
  int fd = qn_net_create_listener(9999, "127.0.0.1");
  if (fd < 0) {
    return -1;
  }
}

qn_server_t *qn_server_create(struct qn_config_t *config) {
  qn_assert(config);
  qn_assert(config->thread_num > 0);
  qn_server_t *server = qn_alloc_type(qn_server_t);
  server->thread_num = config->thread_num;
  server->dispatcher = qn_alloc_type(qn_dispatcher_t);
  server->event = qn_alloc_type(qn_io_event_t);
  if (init_server_event(server) < 0) {
    qn_free(server->event);
    qn_free(server->dispatcher);
    qn_free(server);
    return NULL;
  }

  return server;
}

