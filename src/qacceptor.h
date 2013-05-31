/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QACCEPTOR_H__
#define __QACCEPTOR_H__

#include "qcore.h"
#include "qmailbox.h"

int (*qmsg_func_t)(qmsg_t *msg, void *reader);

struct qacceptor_t {
  qmailbox_t    box;
  qengine_t    *engine;
  qmsg_func_t   handle;
  void         *owner;
};

void  qacceptor_init(qacceptor_t *, qengine_t *, qmsg_fun_t, void *);

#endif  /* __QACCEPTOR_H__ */
