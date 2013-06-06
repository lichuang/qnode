/*
 * See Copyright Notice in qnode.h
 */

#include "qactor.h"
#include "qmmsg.h"

qmsg_t* qmmsg_spawn_new(qactor_t *actor, qactor_t *parent,
                        lua_State *state, qid_t sender, qid_t recver) {
  qmsg_t        *msg;
  qmmsg_spawn_t *spawn;

  msg = qmsg_new(sender, recver, sizeof(qmmsg_spawn_t), M_SPAWN);
  if (msg == NULL) {
    return NULL;
  }

  spawn         = (qmmsg_spawn_t*)msg;
  spawn->aid    = actor->aid;
  spawn->parent = parent->aid;
  spawn->state  = state;
  spawn->actor  = actor;

  qactor_attach(actor, state);
  actor->parent = parent->aid;

  return msg;
}

qmsg_t* qmmsg_signal_new(int signo, qid_t sender, qid_t recver) {
  qmsg_t         *msg;
  qmmsg_signal_t *signal;

  msg = qmsg_new(sender, recver, sizeof(qmmsg_signal_t), M_SIGNAL);
  if (msg == NULL) {
    return NULL;
  }
  signal        = (qmmsg_signal_t*)msg;
  signal->signo = signo;

  return msg;
}
