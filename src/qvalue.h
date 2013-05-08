/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QVALUE_H__
#define __QVALUE_H__

#include "qcore.h"

#define QSTRING_TYPE 0
#define QNUMBER_TYPE 1 

typedef int qnumber_t;

struct qvalue_t {
  unsigned int  type:1;

  union {
    qstring_t str;
    qnumber_t   num;
  } data;
};

#define   qvalue_isnumber(value)  ((value)->type == QNUMBER_TYPE)
#define   qvalue_isstring(value)  ((value)->type == QSTRING_TYPE)

#define   qvalue_number(n)        { .type = QNUMBER_TYPE, {.num = (n)} }
#define   qvalue_string(s)        { .type = QSTRING_TYPE, {.str = (char*)(s)} }

void      qvalue_destroy(qvalue_t *value);
void      qvalue_clone(qvalue_t *value1, qvalue_t *value2);

#endif  /* __QVALUE_H__ */
