/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QWMSG_H__
#define __QWMSG_H__

#include "qmsg.h"
#include "qmmsg.h"

/* W_* means Worker thread handle message */
enum {
  W_START   = 0,
  W_SPAWN   = 1,
  W_SIGNAL  = 2,
  QWMSG_NUM
};

typedef struct qwmsg_start_t {
  qmsg_header_fields;
  qid_t aid;
} qwmsg_start_t;

typedef struct qwmsg_spawn_t {
  qmsg_header_fields;

  qid_t       aid;
  qid_t       parent;
  lua_State  *state;
  qactor_t   *actor;
} qwmsg_spawn_t;

typedef struct qwmsg_signal_t {
  qmsg_header_fields;
  int         signo;
} qwmsg_signal_t;

qmsg_t* qwmsg_start_new(qid_t sender, qid_t recver);

qmsg_t* qwmsg_spawn_new(qactor_t *actor, qactor_t *parent,
                        lua_State *state, qid_t sender, qid_t recver);

qmsg_t* qwmsg_signal_new(qid_t recver, int signo);

#endif  /* __QWMSG_H__ */

