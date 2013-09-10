#include "ctest.h"
#include "qcore.h"
#include "qfreelist.h"

#define DATA_FREE_NUM 5
static qfreelist_t data_freelist;
static int         alloc_num = 0;
static int         free_num  = 0;

typedef struct testdata_t {
  qfree_item_fields;
  int *num;
} testdata_t;

static int  data_init(void *data);
static void data_destroy(void *data);

static void
setup() {
  qfreelist_init(&data_freelist, "data free list",
                 sizeof(testdata_t), DATA_FREE_NUM,
                 data_init, data_destroy);
}

static void
teardown() {
}

static void
freelist_test() {
  int i;
  testdata_t *data;

  CTEST_NUM_EQ(DATA_FREE_NUM, data_freelist.initnum);

  CTEST_FALSE(qlist_empty(&(data_freelist.free)));
  CTEST_NUM_EQ(0, alloc_num);

  for (i = 0; i < DATA_FREE_NUM; ++i) {
    data = qfreelist_alloc(&data_freelist);
  }
  CTEST_NUM_EQ(DATA_FREE_NUM, alloc_num);
  /* after alloc DATA_FREE_NUM items, free list empty */
  CTEST_TRUE(qlist_empty(&(data_freelist.free)));

  qfreelist_free(&data_freelist, (qfree_item_t*)data);
  /* after free an item, free list is not empty */
  CTEST_FALSE(qlist_empty(&(data_freelist.free)));
  CTEST_NUM_EQ(1, free_num);

  qfreelist_alloc(&data_freelist);
  /* after alloc an item, free list is empty */
  CTEST_TRUE(qlist_empty(&(data_freelist.free)));
  CTEST_NUM_EQ(DATA_FREE_NUM + 1, alloc_num);

  /*
   * after alloc an item, freelist will prealloc more items,
   * free list is not empty
   */
  qfreelist_alloc(&data_freelist);
  CTEST_FALSE(qlist_empty(&(data_freelist.free)));
  CTEST_NUM_EQ(DATA_FREE_NUM + 2, alloc_num);

  for (i = 1; i < DATA_FREE_NUM; ++i) {
    CTEST_FALSE(qlist_empty(&(data_freelist.free)));
    qfreelist_alloc(&data_freelist);
  }
  /* after alloc DATA_FREE_NUM - 1 items, free list empty */
  CTEST_TRUE(qlist_empty(&(data_freelist.free)));

  CTEST_NUM_EQ(DATA_FREE_NUM + 2 + DATA_FREE_NUM - 1, alloc_num);

  qfreelist_destroy(&data_freelist);
}

static int
data_init(void *data) {
  testdata_t *t;

  t = (testdata_t*)data;
  t->num = malloc(sizeof(int));
  *(t->num) = 1;
  alloc_num++;

  return QOK;
}

static void
data_destroy(void *data) {
  testdata_t *t;

  t = (testdata_t*)data;
  free(t->num);
  free_num++;
}

static ctest_reg_t reg[] = {
  {"freelist_test", &freelist_test},
  {NULL,            NULL},
};

int main(int argc, char *argv[]) {
  ctest_init(&setup, &teardown);
  ctest_add(reg);;
  ctest_run();

  return 0;
}
