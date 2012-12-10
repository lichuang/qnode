/*
 * See Copyright Notice in qnode.h
 */

#include "qconfig.h"
#include "qengine.h"
#include "qserver.h"

int main(int argc, char *argv[]) {
  struct qconfig_t config;
  config.thread_num = 10;

  qserver_t* server = qserver_create(&config);
  qengine_loop(server->engine);

  return 0;
}
