#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ctest.h"

#define NONE  "\033[m"
#define RED   "\033[0;32;31m"

typedef struct ctest_base_t {
  ctest_t       *head;
  ctest_free_pt  free;
  int            passed;
  int            error;
} ctest_base_t;

static ctest_base_t base = {
  .head = NULL,
  .free = NULL,
  .passed = 0,
  .error = 0
};

static void close_test();
static int  is_num_equal(double expected, double actual);
static int  is_str_equal(const char *expected, const char *actual);

void
ctest_add(ctest_reg_t *reg) {
  int      i;
  ctest_t *test, *current;

  current = NULL;
  for (i = 0; reg[i].name; ++i) {
    test = (ctest_t*)malloc(sizeof(ctest_t));
    if (test == NULL) {
      return;
    }
    test->name = strdup(reg[i].name);
    if (test->name == NULL) {
      return;
    }
    test->func = reg[i].func;
    test->next = NULL;
    if (base.head == NULL) {
      base.head = test;
    }
    if (current == NULL) {
      current = test;
    } else {
      current->next = test;
      current = test;
    }
  }
}

static int
is_num_equal(double expected, double actual) {
  return (abs(expected - actual) < 0.000001);
}

static int
is_str_equal(const char *expected, const char *actual) {
  if (expected == actual) {
    return 1;
  }
  if (strcmp(expected, actual) == 0) {
    return 1;
  }
  return 0;
}

void
ctest_num_eq(double expected, double actual,
             const char *file, const char *func, int line) {
  if (!is_num_equal(expected, actual)) {
    printf(RED"in %s:%d %s expected<%f> != actual<%f>\n"NONE, file, line, func, expected, actual);
    base.error++;
  } else {
    base.passed++;
  }
}

void
ctest_num_ne(double expected, double actual,
             const char *file, const char *func, int line) {
  if (is_num_equal(expected, actual)) {
    printf(RED"in %s:%d %s expected<%f> == actual<%f>\n"NONE, file, line, func, expected, actual);
    base.error++;
  } else {
    base.passed++;
  }
}

void ctest_str_eq(const char *expected, const char *actual,
                  const char *file, const char *func, int line) {
  if (!is_str_equal(expected, actual)) {
    printf(RED"in %s:%d %s expected<%s> != actual<%s>\n"NONE, file, line, func, expected, actual);
    base.error++;
  } else {
    base.passed++;
  }
}

void
ctest_str_ne(const char *expected, const char *actual,
             const char *file, const char *func, int line) {
  if (is_str_equal(expected, actual)) {
    printf(RED"in %s:%d %s expected<%s> == actual<%s>\n"NONE, file, line, func, expected, actual);
    base.error++;
  } else {
    base.passed++;
  }
}

void
ctest_true(int condition, const char *file,
           const char *func, int line) {
  if (!condition) {
    printf(RED"in %s:%d %s assert false\n"NONE, file, line, func);
    base.error++;
  } else {
    base.passed++;
  }
}

void
ctest_false(int condition, const char *file,
            const char *func, int line) {
  if (condition) {
    printf(RED"in %s:%d %s assert true\n"NONE, file, line, func);
    base.error++;
  } else {
    base.passed++;
  }
}

void ctest_run() {
  ctest_t *test;

  for (test = base.head; test; test = test->next) {
    test->func();
  }

  close_test();
}

static
void close_test() {
  ctest_t *test, *next;
  int      num;

  for (test = base.head, num = 0; test; test = next) {
    next = test->next;
    free(test->name);
    free(test);    
    num++;
  }

  printf("Report:\n");
  printf("test unit: %d, test cases: %d\n", num, base.passed + base.error);
  printf("passed: %d, fail: %d\n", base.passed, base.error);

  if (base.free) {
    base.free();
  }
}

void
ctest_init(ctest_free_pt free) {
  base.free = free;
}
