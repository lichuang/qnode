/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include <unistd.h>
#include "qconfig.h"
#include "qengine.h"
#include "qserver.h"

static void
usage() {
  printf("\nUsage: qserver [option]\n"
    "options:\n"
    "\t-c config file\n"
    "\t-h print usage\n");
}

int
main(int argc, char *argv[]) {
  int         opt;
  const char *file;
  qconfig_t   config;

  file = "./etc/config.lua";
  while ((opt = getopt(argc, argv, "c:h")) != -1) {
    switch (opt) {
    case 'h':
      usage();
      return 0;
      break;
    case 'c':
      file = optarg; 
      break;
    }   
  }

  qconfig_init(&config, file);

  qserver_run(&config);

  return 0;
}
