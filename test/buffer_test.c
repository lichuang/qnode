#include <string.h>
#include "ctest.h"
#include "qcore.h"
#include "qbuffer.h"

static void
buffer_test() {
  int        len;
  qbuffer_t *buffer;
  const char *str = "hello";

  qbuffer_init_freelist();

  buffer = qbuffer_new();
  CTEST_NUM_EQ(1, buffer->active);
  CTEST_NUM_EQ(1, buffer->flag);
  len = qbuffer_wlen(buffer);
  CTEST_NUM_EQ(0, qbuffer_rlen(buffer));
  qbuffer_write(buffer, str, strlen(str));
  CTEST_NUM_EQ(strlen(str), qbuffer_rlen(buffer));
  CTEST_NUM_EQ(len - strlen(str), qbuffer_wlen(buffer));
  CTEST_NUM_EQ(0, strcmp(str, qbuffer_read(buffer, strlen(str))));
  CTEST_NUM_EQ(0, qbuffer_rlen(buffer));
  CTEST_NUM_EQ(len - strlen(str), qbuffer_wlen(buffer));

  qbuffer_extend(buffer, len + strlen(str));
  CTEST_NUM_EQ(((len + strlen(str) + 127) & 0xFF80) - strlen(str), qbuffer_wlen(buffer));

  qbuffer_free(buffer);
  CTEST_NUM_EQ(len, qbuffer_wlen(buffer));
  CTEST_NUM_EQ(0, buffer->active);
  CTEST_NUM_EQ(1, buffer->flag);

  qbuffer_destroy_freelist();
}

static ctest_reg_t reg[] = {
  {"buffer_test",  &buffer_test},
  {NULL,            NULL},
};

int
main(int argc, char *argv[]) {
  ctest_init(NULL, NULL);
  ctest_add(reg);;
  ctest_run();
  return 0;
}
