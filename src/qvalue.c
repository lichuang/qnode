/*
 * See Copyright Notice in qnode.h
 */

#include "qalloc.h"
#include "qstring.h"
#include "qvalue.h"

void
qvalue_destroy(qvalue_t *value) {
  if (value->type == QSTRING_TYPE) {
    qstring_destroy(value->data.str);
  }
}

void
qvalue_clone(qvalue_t *value1, qvalue_t *value2) {
  if (qvalue_isstring(value2)) {
    if (!qvalue_isstring(value1)) {
      value1->data.str = qstring_new(value2->data.str);
      value1->type = QSTRING_TYPE;
    } else {
      value1->data.str = qstring_assign(value1->data.str,
                                        value2->data.str);
    }

    return;
  }

  if (qvalue_isstring(value1)) {
    qstring_destroy(value1->data.str);
  }
  if (qvalue_isnumber(value2)) {
    value1->type = QNUMBER_TYPE;
    value1->data.num = value2->data.num;
  } else {
    value1->type = QDATA_TYPE;
    value1->data.data = value2->data.data;
  }
}

void
qvalue_newstr(qvalue_t *value, const char *str) {
  value->data.str = qstring_new(str);
  if (value->data.str == NULL) {
    return;
  }
  value->type = QSTRING_TYPE;
}

void
qvalue_newnum(qvalue_t *value, qnumber_t num) {
  value->type = QNUMBER_TYPE;
  value->data.num = num;
}
