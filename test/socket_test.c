#include <string.h>
#include "ctest.h"
#include "qcore.h"
#include "qsocket.h"

static void
socket_test() {
}

static ctest_reg_t reg[] = {
  {"socket_test",  &socket_test},
  {NULL,            NULL},
};

int main(int argc, char *argv[]) {
  ctest_init(NULL, NULL);
  ctest_add(reg);;
  ctest_run();
  return 0;
}
