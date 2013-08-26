/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include <unistd.h>
#include "qconfig.h"
#include "qengine.h"
#include "qlog.h"
#include "qserver.h"

static void
usage() {
  printf("Usage: qserver [option]\n"
    "options:\n"
    "\t-c config file\n"
    "\t-h print usage\n");
}

int
main(int argc, char *argv[]) {
  int         opt;
  const char *file;

  file = NULL;
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

  if (file == NULL) {
    qstdout("no config file\n");
    usage();
    return -1;
  }

  if (qconfig_init(file) < 0) {
    qstdout("qconfig_init error\n");
    return -1;
  }

  qserver_run();

  return 0;
}
