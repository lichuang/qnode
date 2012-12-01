/*
 * See Copyright Notice in qnode.h
 */

#include "qconfig.h"
#include "qevent.h"
#include "qserver.h"

int main(int argc, char *argv[]) {
  qnode_config_t config;
  config.thread_num = 10;

  qnode_server_t *server = qnode_server_create(&config);
  while (qnode_dispatcher_run(server->dispatcher) > 0) {
  }

  return 0;
}
