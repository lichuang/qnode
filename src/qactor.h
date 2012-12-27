/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QACTOR_H__
#define __QACTOR_H__

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "qtype.h"
#include "qlist.h"

struct qserver_t;
struct qthread_t;

typedef struct qactor_t {
  lua_State *state;
  struct qthread_t *thread;
  qaid_t aid;
  qaid_t parent;
  qlist_t entry;
} qactor_t;

qid_t qactor_new_id();
qactor_t *qactor_new(qaid_t aid);
void qactor_destroy(qactor_t *actor);

/* spawn an actor as child, return the actor ID */
qaid_t qactor_spawn(qactor_t *actor, lua_State *state);

#endif  /* __QACTOR_H__ */
