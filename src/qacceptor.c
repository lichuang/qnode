/*
 * See Copyright Notice in qnode.h
 */

#include "qacceptor.h"
#include "qmailbox.h"
#include "qmsg.h"

void 
qacceptor_init(qacceptor_t *acceptor, qengine_t *engine,
               qmsg_func_t *func, void *owner) {
  acceptor->engine = engine;
  acceptor->handle = func;
  acceptor->owner  = owner;
  qmailbox_init(&(acceptor->box), acceptor);
}
