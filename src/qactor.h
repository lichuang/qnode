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

struct qdict_t;
struct qengine_t;
struct qserver_t;
struct qthread_t;

typedef struct qactor_t {
  lua_State *state;
  qtid_t tid;
  qid_t aid;
  qid_t parent;
  qlist_t entry;
  qlist_t desc_list;
  struct qdict_t *listen_params;
} qactor_t;

qid_t qactor_new_id();
qactor_t *qactor_new(qid_t aid);
void qactor_attach(qactor_t *actor, lua_State *state);
void qactor_destroy(qactor_t *actor);

/* spawn an actor as child, return the actor ID */
qid_t qactor_spawn(qactor_t *actor, lua_State *state);

struct qengine_t* qactor_get_engine(qactor_t *actor);

#endif  /* __QACTOR_H__ */
