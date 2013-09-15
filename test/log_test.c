#include "ctest.h"
#include "qcore.h"
#include "qlog.h"
#include "qfreelist.h"

static void
log_test() {
  qlog_t *log1, *log2;

  qlog_init_free_list();

  log1 = qlog_new();
  CTEST_TRUE(qlist_empty(&log1->fentry));
  log2 = qlog_new();
  CTEST_TRUE(qlist_empty(&log2->fentry));
  qlist_add_tail(&(log1->fentry), &(log2->fentry));

  qlog_free(&(log1->fentry));

  qlog_destroy_free_list();
}

static ctest_reg_t reg[] = {
  {"log_test",      &log_test},
  {NULL,            NULL},
};

int main(int argc, char *argv[]) {
  ctest_init(NULL, NULL);
  ctest_add(reg);;
  ctest_run();

  return 0;
}
