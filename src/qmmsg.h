/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QMMSG_H__
#define __QMMSG_H__

#include "qactor.h"
#include "qmsg.h"

/* M_* means Main thread handle message */
enum {
  M_SPAWN   = 0,
  M_SIGNAL  = 1,
  QMMSG_NUM
};

typedef struct qmmsg_spawn_t {
  qmsg_header_fields;

  qid_t       aid;
  qid_t       parent;
  lua_State  *state;
  qactor_t   *actor;
} qmmsg_spawn_t;

typedef struct qmmsg_signal_t {
  qmsg_header_fields;

  int         signo;
} qmmsg_signal_t;

qmsg_t* qmmsg_spawn_new(qactor_t *actor, qactor_t *parent,
                        lua_State *state, qid_t sender, qid_t recver);

qmsg_t* qmmsg_signal_new(int signo, qid_t sender, qid_t recver);

#endif  /* __QMMSG_H__ */
