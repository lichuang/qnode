/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QACCEPTOR_H__
#define __QACCEPTOR_H__

#include "qcore.h"
#include "qmailbox.h"

struct qacceptor_t {
  qmailbox_t    box;
  qengine_t    *engine;
  qmsg_func_t  *handle;
  void         *owner;
};

void qacceptor_init(qacceptor_t *acceptor, qengine_t *engine,
                    qmsg_func_t *func, void *reader);

#endif  /* __QACCEPTOR_H__ */
