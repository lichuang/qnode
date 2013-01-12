/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QCONNECTION_H__
#define __QCONNECTION_H__

#include "qlist.h"
#include "qtype.h"

typedef struct qconnection_t {
  qlist_t entry;
  int fd;
  qid_t aid;
} qconnection_t;

qconnection_t* qconnection_get(int fd);

#endif  /* __QCONNECTION_H__ */
