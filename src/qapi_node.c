/*
 * See Copyright Notice in qnode.h
 */
#include <stdio.h>
#include "qactor.h"
#include "qassert.h"
#include "qdefines.h"
#include "qdict.h"
#include "qluautil.h"
#include "qlog.h"
#include "qmsg.h"
#include "qserver.h"
#include "qstring.h"
#include "qthread.h"

/*
 * spawn an actor, return the actor ID
 * */
static int qnode_spawn(lua_State *state) {
  qactor_t *actor = qlua_get_actor(state);
  const char *mod = lua_tostring(state, 1);
  const char *fun = lua_tostring(state, 2);
  qthread_t *thread = g_server->threads[actor->tid];
  lua_State *new_state = qlua_new_thread(thread);
  qstring_t string;
  qstring_init(&string);
  qstring_assign(&string, mod);
  qstring_append(&string, ".lua");
  if (qlua_dofile(new_state, string.data) != 0) {
    qerror("load script to launce error");
  }
  qstring_destroy(&string);

  /* copy args table */
  qlua_copy_table(state, new_state, 3);

  lua_getglobal(new_state, mod);
  lua_getfield(new_state, -1, fun);
  /* push the args table */
  lua_pushvalue(new_state, 1);
  return (int)qactor_spawn(actor, new_state);
}

static int qnode_send(lua_State *state) {
  qactor_t *srqnode = qlua_get_actor(state);
  qid_t id = (qid_t)lua_tonumber(state, 1);
  qactor_t *dst_actor = qserver_get_actor(id);
  if (dst_actor == NULL) {
    return 0;
  }
  /* copy args table */
  qactor_msg_t *actor_msg = qlua_copy_arg_table(state, 2);

  qmsg_t *msg = qmsg_new(srqnode->tid, dst_actor->tid);
  qmsg_init_tsend(msg, actor_msg);
  qmsg_send(msg);
  return 0;
}

luaL_Reg node_apis[] = {
  {"qnode_spawn",       qnode_spawn},
  {"qnode_send",        qnode_send},
  {NULL, NULL},
};
