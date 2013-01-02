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
  qid_t aid;
  qid_t parent;
  qlist_t entry;
} qactor_t;

qid_t qactor_new_id();
qactor_t *qactor_new(qid_t aid, lua_State *state);
void qactor_destroy(qactor_t *actor);

/* spawn an actor as child, return the actor ID */
qid_t qactor_spawn(qactor_t *actor, lua_State *state);

#endif  /* __QACTOR_H__ */
