/*
 * See Copyright Notice in qnode.h
 */

#include "qalloc.h"
#include "qstring.h"
#include "qvalue.h"

qvalue_t* qvalue_newnum(qnumber_t num) {
  qvalue_t *value;

  value = qalloc(sizeof(qvalue_t));
  if (value == NULL) {
    return NULL;
  }

  value->type = QNUMBER_TYPE;
  value->data.num = num;

  return value;
}

qvalue_t* qvalue_newstr(const char *str) {
  qvalue_t *value;

  value = qalloc(sizeof(qvalue_t));
  if (value == NULL) {
    return NULL;
  }

  value->type = QSTRING_TYPE;
  value->data.str = qstring_new(str);
  if (value->data.str == NULL) {
    qfree(value);
    return NULL;
  }

  return value;
}

void qvalue_destroy(qvalue_t *value) {
  if (value->type == QSTRING_TYPE) {
    qstring_destroy(value->data.str);
  }
  qfree(value);
}
