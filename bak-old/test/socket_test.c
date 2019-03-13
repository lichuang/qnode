#include <string.h>
#include "ctest.h"
#include "qactor.h"
#include "qcore.h"
#include "qsocket.h"

#undef DEBUG
#include "qworker.h"

static void
socket_test() {
  test_flag = 1;

  qsocket_t *socket;
  qactor_t  *actor;

  qbuffer_init_freelist();
  qsocket_init_free_list();

  actor = qactor_new(1);
  socket = qsocket_new(1, actor);

  CTEST_TRUE(socket->in != NULL);
  CTEST_TRUE(socket->out != NULL);
  CTEST_TRUE(!qlist_empty(&(socket->entry)));

  qsocket_free(socket);
  CTEST_TRUE(qlist_empty(&(socket->entry)));
  CTEST_TRUE(socket->in == NULL);
  CTEST_TRUE(socket->out == NULL);

  qsocket_destroy_free_list();
  qbuffer_destroy_freelist();
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
