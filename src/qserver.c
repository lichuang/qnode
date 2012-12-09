/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qassert.h"
#include "qconfig.h"
#include "qengine.h"
#include "qlog.h"
#include "qmalloc.h"
#include "qmailbox.h"
#include "qnet.h"
#include "qserver.h"
#include "qthread.h"

static void server_accept(int fd, int flags, void *data) {
  qnode_info("add a connection....")
}

static int init_server_event(struct qnode_server_t *server) {
  int fd = qnode_net_tcp_server(22222, "127.0.0.1");
  if (fd < 0) {
    return -1;
  }

  qnode_engine_t *engine = server->engine;
  if (qnode_engine_add_event(engine, fd, QNODE_EVENT_READ, server_accept, server) < 0) {
    return -1;
  }
  return 0;
}

static void server_box(int fd, int flags, void *data) {
}

qnode_server_t *qnode_server_create(struct qnode_config_t *config) {
  qnode_assert(config);
  qnode_assert(config->thread_num > 0);
  qnode_server_t *server = qnode_alloc_type(qnode_server_t);
  server->thread_num = config->thread_num;
  server->engine = qnode_engine_new();
  if (init_server_event(server) < 0) {
    qnode_engine_destroy(server->engine);
    qnode_free(server);
    return NULL;
  }
  /* alloc threads and mailboxs */
  server->threads = (qnode_thread_t**)qnode_malloc(server->thread_num * sizeof(qnode_thread_t*));
  qnode_alloc_assert(server->threads);
  server->threads[0] = NULL;
  server->boxs = (qnode_mailbox_t**)qnode_malloc(server->thread_num * sizeof(qnode_mailbox_t*));
  qnode_alloc_assert(server->boxs);
  server->boxs[0] = NULL;
  server->thread_boxs = (qnode_mailbox_t**)qnode_malloc(server->thread_num * sizeof(qnode_mailbox_t*));
  qnode_alloc_assert(server->thread_boxs);
  server->thread_boxs[0] = NULL;
  int i;
  for (i = 1; i <= server->thread_num; ++i) {
    server->boxs[i] = qnode_mailbox_new(server_box, server);
    qnode_assert(server->boxs[i]);
    server->threads[i] = qnode_thread_new(server, i); 
    qnode_assert(server->threads[i]);
    server->thread_boxs[i] = qnode_thread_mailbox(server->threads[i]);
  }
  qnode_info("qserver started...");
  return server;
}

qnode_mailbox_t *qnode_server_get_box(qnode_server_t *server, int tid) {
  return server->boxs[tid];
}
