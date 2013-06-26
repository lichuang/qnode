/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QHANDLER_H__
#define __QHANDLER_H__

#include "qcore.h"

typedef struct qhandler_t {
  qmsg_func_t   handler;
  void         *reader;
} qhandler_t;

typedef struct qhandler_manager_t {
  handler* handlers[];
  qid_t    current;
} qhandler_manager_t;

void        qhandler_manager_init(qhandler_manager_t *);
qid_t       qhandler_add(qhandler_manager_t *, qhandler_t *);
void        qhandler_delete(qhandler_manager_t *,qid_t);
qhandler_t* qhandler_get(qhandler_manager_t *, qid_t id);

#endif  /* __QHANDLER_H__ */
