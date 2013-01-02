/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qconfig.h"
#include "qengine.h"
#include "qserver.h"

int main(int argc, char *argv[]) {
  struct qconfig_t config;
  config.thread_num = 10;
  if (argc > 1) {
    qconfig_init(&config, argv[1]);
  } else {
    qconfig_init(&config, NULL);
  }

  qserver_run(&config);

  return 0;
}
