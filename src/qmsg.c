/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qassert.h"
#include "qmalloc.h"
#include "qmsg.h"

qmsg_t* qmsg_new() {
  qmsg_t *msg = qalloc_type(qmsg_t);
  if (msg == NULL) {
    return NULL;
  }
  return msg;
}
