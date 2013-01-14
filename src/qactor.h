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

#define QINVALID_LUA_REF -1
enum lua_api_ref {
  LISTENER = 0,
  QMAX_LUA_API_REF
};

typedef struct qactor_t {
  lua_State *state;
  qtid_t tid;
  qid_t aid;
  qid_t parent;
  qlist_t entry;
  int listen_fd;
  qlist_t conn_list;
  int lua_ref[QMAX_LUA_API_REF];
} qactor_t;

qid_t qactor_new_id();
qactor_t *qactor_new(qid_t aid);
void qactor_attach(qactor_t *actor, lua_State *state);
void qactor_destroy(qactor_t *actor);

/* spawn an actor as child, return the actor ID */
qid_t qactor_spawn(qactor_t *actor, lua_State *state);

void qactor_accept(int fd, int flags, void *data);

#endif  /* __QACTOR_H__ */
