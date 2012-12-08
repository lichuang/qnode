/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qassert.h"
#include "qconfig.h"
#include "qengine.h"
#include "qlog.h"
#include "qmalloc.h"
#include "qnet.h"
#include "qserver.h"

static void server_accept(int fd, int flags, void *data) {
    qnode_info("add a connection....")
}

static int init_server_event(struct qnode_server_t *server) {
    int fd = qnode_net_create_listener(22222, "127.0.0.1");
    if (fd < 0) {
        return -1;
    }
   
    qnode_engine_t *engine = server->engine;
    if (qnode_engine_add_event(engine, fd, QNODE_EVENT_READ, server_accept, server) < 0) {
        return -1;
    }
    return 0;
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

    qnode_info("qserver started...");
    return server;
}

