/*
 * See Copyright Notice in qnode.h
 */

#include "qn_config.h"
#include "qn_event.h"
#include "qn_server.h"

int main(int argc, char *argv[]) {
  qn_config_t config;
  config.thread_num = 10;

  qn_server_t *server = qn_server_create(&config);
  while (qn_dispatcher_run(server->dispatcher) > 0) {
  }

  return 0;
}
