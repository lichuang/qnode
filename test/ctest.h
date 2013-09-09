#ifndef __CTEST_H__
#define __CTEST_H__

#ifdef _cplusplus
extern "C" {
#endif

typedef void (*ctest_pt_t)(void);
typedef void (*ctest_free_pt)(void);

typedef struct ctest_t ctest_t;

struct ctest_t {
  ctest_t     *next;
  char        *name;
  ctest_pt_t   func;
};

typedef struct ctest_reg_t {
  const char  *name;
  ctest_pt_t   func;
} ctest_reg_t;

void ctest_init(ctest_free_pt free);
void ctest_add(ctest_reg_t *reg);
void ctest_run();

void ctest_num_eq(double expected, double actual,
                  const char *file, const char *func, int line);
void ctest_num_ne(double expected, double actual,
                  const char *file, const char *func, int line);
void ctest_str_eq(const char *expected, const char *actual,
                  const char *file, const char *func, int line);
void ctest_str_ne(const char *expected, const char *actual,
                  const char *file, const char *func, int line);
void ctest_true(int condition, const char *file,
                const char *func, int line);
void ctest_false(int condition, const char *file,
                 const char *func, int line);

#define CTEST_NUM_EQ(expected, actual)                                  \
  do {                                                                  \
    ctest_num_eq(expected, actual, __FILE__, __FUNCTION__, __LINE__);   \
  } while (0)

#define CTEST_NUM_NE(expected, actual)                                  \
  do {                                                                  \
    ctest_num_ne(expected, actual, __FILE__, __FUNCTION__, __LINE__);   \
  } while (0)

#define CTEST_STR_EQ(expected, actual)                                  \
  do {                                                                  \
    ctest_str_eq(expected, actual, __FILE__, __FUNCTION__, __LINE__);   \
  } while (0)

#define CTEST_STR_NE(expected, actual)                                  \
  do {                                                                  \
    ctest_str_ne(expected, actual, __FILE__, __FUNCTION__, __LINE__);   \
  } while (0)

#define CTEST_TRUE(condition)                                           \
  do {                                                                  \
    ctest_true(condition, __FILE__, __FUNCTION__, __LINE__);            \
  } while (0)

#define CTEST_FALSE(condition)                                          \
  do {                                                                  \
    ctest_false(condition, __FILE__, __FUNCTION__, __LINE__);           \
  } while (0)

#ifdef _cplusplus
}
#endif

#endif  /* __CTEST_H__ */
