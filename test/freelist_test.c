#include "ctest.h"
#include "qcore.h"
#include "qfreelist.h"

#define DATA_FREE_NUM 5
static int         alloc_num = 0;
static int         free_num  = 0;

typedef struct testdata_t {
  qfreeitem_fields;
  int num;
} testdata_t;

static int  data_init(void *data);
static int  data_init_fail(void *data);
static void data_destroy(void *data);

static void
setup() {
}

static void
teardown() {
}

static void
freelist_test_fail() {
  int i;
  testdata_t *data;
  qfreelist_t data_freelist;

  alloc_num = free_num = 0;

  qfreelist_conf_t conf = QFREELIST_CONF("data free list",
                                         sizeof(testdata_t),
                                         DATA_FREE_NUM,
                                         data_init_fail,
                                         data_destroy, NULL);
  qfreelist_init(&data_freelist, &conf);
  CTEST_NUM_EQ(DATA_FREE_NUM, data_freelist.initnum);
  for (i = 0; i < DATA_FREE_NUM; ++i) {
    data = qfreelist_new(&data_freelist);
    CTEST_TRUE(data == NULL);
  }
  CTEST_NUM_EQ(DATA_FREE_NUM, alloc_num);
  /* after alloc DATA_FREE_NUM items fail, free list not empty */
  CTEST_FALSE(qlist_empty(&(data_freelist.free)));

  qfreelist_destroy(&data_freelist);
}

static void
freelist_test() {
  int i;
  testdata_t *data;
  qfreelist_t data_freelist;

  alloc_num = free_num = 0;
  qfreelist_conf_t conf = QFREELIST_CONF("data free list",
                                         sizeof(testdata_t),
                                         DATA_FREE_NUM,
                                         data_init,
                                         data_destroy, NULL);
  qfreelist_init(&data_freelist, &conf);
  CTEST_NUM_EQ(DATA_FREE_NUM, data_freelist.initnum);
  CTEST_TRUE(!qlist_empty(&(data_freelist.free)));
  CTEST_TRUE(qlist_empty(&(data_freelist.alloc)));

  CTEST_FALSE(qlist_empty(&(data_freelist.free)));
  CTEST_NUM_EQ(0, alloc_num);

  for (i = 0; i < DATA_FREE_NUM; ++i) {
    data = qfreelist_new(&data_freelist);
    CTEST_NUM_EQ(1, data->active);
    CTEST_NUM_EQ(1, data->flag);
  }
  CTEST_TRUE(qlist_empty(&(data_freelist.free)));
  CTEST_TRUE(!qlist_empty(&(data_freelist.alloc)));
  CTEST_NUM_EQ(DATA_FREE_NUM, alloc_num);
  /* after alloc DATA_FREE_NUM items, free list empty */
  CTEST_TRUE(qlist_empty(&(data_freelist.free)));

  qfreelist_free(&data_freelist, (qfreeitem_t*)data);
  CTEST_NUM_EQ(0, data->active);
  /* after free an item, free list is not empty */
  CTEST_FALSE(qlist_empty(&(data_freelist.free)));
  CTEST_NUM_EQ(1, free_num);

  qfreelist_new(&data_freelist);
  /* after alloc an item, free list is empty */
  CTEST_TRUE(qlist_empty(&(data_freelist.free)));
  CTEST_NUM_EQ(DATA_FREE_NUM + 1, alloc_num);

  /*
   * after alloc an item, freelist will prealloc more items,
   * free list is not empty
   */
  qfreelist_new(&data_freelist);
  CTEST_FALSE(qlist_empty(&(data_freelist.free)));
  CTEST_NUM_EQ(DATA_FREE_NUM + 2, alloc_num);

  for (i = 1; i < DATA_FREE_NUM; ++i) {
    CTEST_FALSE(qlist_empty(&(data_freelist.free)));
    qfreelist_new(&data_freelist);
  }
  /* after alloc DATA_FREE_NUM - 1 items, free list empty */
  CTEST_TRUE(qlist_empty(&(data_freelist.free)));

  CTEST_NUM_EQ(DATA_FREE_NUM + 2 + DATA_FREE_NUM - 1, alloc_num);

  qfreelist_destroy(&data_freelist);

  CTEST_TRUE(qlist_empty(&(data_freelist.free)));
  CTEST_TRUE(qlist_empty(&(data_freelist.alloc)));
}

static int
data_init_fail(void *data) {
  ++alloc_num;
  return QERROR;
}

static int
data_init(void *data) {
  testdata_t *t;

  t = (testdata_t*)data;
  t->num = 1;
  alloc_num++;

  return QOK;
}

static void
data_destroy(void *data) {
  testdata_t *t;

  t = (testdata_t*)data;
  free_num++;
}

static ctest_reg_t reg[] = {
  {"freelist_test",      &freelist_test},
  {"freelist_test_fail", &freelist_test_fail},
  {NULL,            NULL},
};

int main(int argc, char *argv[]) {
  ctest_init(&setup, &teardown);
  ctest_add(reg);;
  ctest_run();

  return 0;
}
