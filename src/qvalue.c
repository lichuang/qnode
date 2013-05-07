/*
 * See Copyright Notice in qnode.h
 */

#include "qalloc.h"
#include "qstring.h"
#include "qvalue.h"

void qvalue_destroy(qvalue_t *value) {
  if (value->type == QSTRING_TYPE) {
    qstring_destroy(value->data.str);
  }
}
