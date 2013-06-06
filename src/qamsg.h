/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QAMSG_H__
#define __QAMSG_H__

#include <lua.h>
#include "qmsg.h"

/* A_* means Actor handle message */
enum {
  A_SEND    = 0,
  QAMSG_NUM
};

typedef struct qactor_msg_t {
  qdict_t *arg_dict;
  qlist_t  entry;
} qactor_msg_t;

typedef struct qamsg_send_t {
  qmsg_header_fields;

  qactor_msg_t actor_msg;
} qamsg_send_t;

qmsg_t* qamsg_send_new(lua_State *state, qid_t sender, qid_t recver); 

#endif  /* __QAMSG_H__ */
