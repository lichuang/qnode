/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QVALUE_H__
#define __QVALUE_H__

#include "qcore.h"

#define QSTRING_TYPE 1
#define QNUMBER_TYPE 2

typedef int qnumber_t;

struct qvalue_t {
  unsigned int  type:1;

  union {
    qstring_t   str;
    qnumber_t   num;
  } data;
};

#define qvalue_isnumber(value) ((value).type == QNUMBER_TYPE)
#define qvalue_isstring(value) ((value).type == QSTRING_TYPE)

qvalue_t* qvalue_newnum(qnumber_t num);
qvalue_t* qvalue_newstr(const char *str);
void      qvalue_destroy(qvalue_t *value);

#endif  /* __QVALUE_H__ */
