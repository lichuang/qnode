/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QAMSG_H__
#define __QAMSG_H__

#include <lua.h>
#include "qmsg.h"

/* A_* means Actor handle message */
enum {
  A_MSG    = 0,
  QAMSG_NUM
};

typedef struct qactor_msg_t {
  qdict_t *arg_dict;
  qlist_t  entry;
} qactor_msg_t;

#define qamsg_header_fields    \
  int             type;        \
  qid_t           src;         \
  qid_t           dst 

typedef struct qamsg_header_t {
  qamsg_header_fields;
} qamsg_header_t;

typedef struct qamsg_t {
  qamsg_header_fields;

  qactor_msg_t actor_msg;
} qamsg_t;

typedef int (qamsg_func_t)(qamsg_header_t *msg, qactor_t *actor);
extern qamsg_func_t* actor_msg_handlers[];

qmsg_t* qamsg_msg_new(lua_State *state, qid_t src, qid_t dst); 

#endif  /* __QAMSG_H__ */
