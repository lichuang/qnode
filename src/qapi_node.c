/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qactor.h"
#include "qamsg.h"
#include "qassert.h"
#include "qdefines.h"
#include "qdict.h"
#include "qluautil.h"
#include "qlog.h"
#include "qmsg.h"
#include "qmutex.h"
#include "qserver.h"
#include "qsocket.h"
#include "qstring.h"
#include "qworker.h"

static int qlnode_spawn(lua_State *state);
static int qlnode_send(lua_State *state);
static int qlnode_recv(lua_State *state);
static int qlnode_attach(lua_State *state);
static int qlnode_self(lua_State *state);
static int qlnode_exit(lua_State *state);

luaL_Reg node_apis[] = {
  {"qlnode_spawn",       qlnode_spawn},
  {"qlnode_send",        qlnode_send},
  {"qlnode_recv",        qlnode_recv},
  {"qlnode_attach",      qlnode_attach},
  {"qlnode_self",        qlnode_self},
  {"qlnode_exit",        qlnode_exit},
  {NULL, NULL},
};

/* spawn an actor, return the actor ID */
static int
qlnode_spawn(lua_State *state) {
  int         id, type;
  const char *mod, *fun;
  qactor_t   *actor;
  qworker_t  *worker;
  lua_State  *new_state;
  qstring_t   string;

  actor = qlua_get_actor(state);
  mod = lua_tostring(state, 1);
  fun = lua_tostring(state, 2);
  worker = workers[actor->tid];
  new_state = qlua_new_thread(worker);

  if (!mod) {
    lua_pushnil(state);
    lua_pushliteral(state, "mod nil");
    return 2;
  }

  if (!fun) {
    lua_pushnil(state);
    lua_pushliteral(state, "func nil");
    return 2;
  }

  if (!new_state) {
    lua_pushnil(state);
    lua_pushliteral(state, "create lua thread error");
    return 2;
  }

  string = qstring_new(mod);
  if (string == NULL) {
    lua_pushnil(state);
    lua_pushliteral(state, "spawn error");
    return 2;
  }
  string = qstring_append(string, ".lua");
  if (string == NULL) {
    lua_pushnil(state);
    lua_pushliteral(state, "spawn error");
    return 2;
  }
  if (qlua_threadloadfile(actor, new_state, string) != 0) {
    lua_pushnil(state);
    lua_pushfstring(state, "load file %s error", string);
    return 2;
  }
  qstring_destroy(string);

  /* copy args table */
  if (qlua_copy_state_table(state, new_state, 3) != QOK) {
    lua_pushnil(state);
    lua_pushliteral(state, "copy arg table error");
    return 2;
  }

  /* get mod::func */
  lua_getglobal(new_state, mod);
  lua_getfield(new_state, -1, fun);
  type = lua_type(new_state, -1);
  if (type != LUA_TFUNCTION) {
    lua_pushnil(state);
    lua_pushfstring(state, "%s::%s not a function, type:%s",
                    mod, fun, lua_typename(new_state, type));
    return 2;
  }

  /* push the args table */
  lua_pushvalue(new_state, 1);
  id = qactor_spawn(actor, new_state);
  if (id == -1) {
    lua_pushnil(state);
    lua_pushliteral(state, "new actor id error");
    return 2;
  }

  lua_pushnumber(state, id);

  return 1;
}

static int
qlnode_send(lua_State *state) {
  qid_t     id, aid;
  qactor_t *actor;
  qmsg_t   *msg;

  actor = qlua_get_actor(state);
  id    = (qid_t)lua_tonumber(state, 1);
  aid   = decode_id(id);

  if (aid >= MAX_ID) {
    lua_pushnil(state);
    lua_pushfstring(state, "send aid: %d error", id);
    return 2;
  }

  msg = qamsg_msg_new(state, actor->aid, id);
  if (msg == NULL) {
    lua_pushnil(state);
    lua_pushliteral(state, "create msg error");
    return 2;
  }
  qworker_send(msg);
  lua_pushnumber(state, 0);

  return 1;
}

static int
qlnode_recv(lua_State *state) {
  qactor_t      *actor;
  qactor_msg_t  *msg;

  actor = qlua_get_actor(state);
  /* if msg list is empty, YIELD the Lua coroutine */
  if (qlist_empty(&(actor->msg_list))) {
    actor->waiting_msg = 1;
    return lua_yield(state, 0); 
  }
  actor->waiting_msg = 0;
  msg = qlist_entry(actor->msg_list.next, qactor_msg_t, entry); 
  qlist_del_init(&(msg->entry));
  lua_newtable(state);
  qlua_dump_dict(state, msg->arg_dict);
  qdict_free(msg->arg_dict);

  return 1;
}

static int
qlnode_attach(lua_State *state) {
  qsocket_t *socket;
  qactor_t  *old_actor, *actor;

  socket = (qsocket_t*)lua_touserdata(state, 1);
  if (socket == NULL) {
    lua_pushnil(state);
    lua_pushliteral(state, "attach socket nil");
    return 2;
  }

  old_actor = qactor_get(socket->aid);
  if (old_actor == NULL) {
    lua_pushnil(state);
    lua_pushliteral(state, "actor nil");
    return 2;
  }
  actor = qlua_get_actor(state);

  if (old_actor->aid == actor->aid) {
    return 0;
  }

  /* detach from old actor */
  qspinlock_lock(&(old_actor->sock_list_lock));
  qlist_del_init(&(socket->entry));
  qspinlock_unlock(&(old_actor->sock_list_lock));

  /* attach to new actor */
  qspinlock_lock(&(actor->sock_list_lock));
  socket->aid = actor->aid;
  qlist_add_tail(&socket->entry, &actor->sock_list);
  qspinlock_unlock(&(actor->sock_list_lock));

  return 0;
}

static int
qlnode_self(lua_State *state) {
  qactor_t *actor;

  actor = qlua_get_actor(state);
  lua_pushnumber(state, actor->aid);

  return 1;
}

static int
qlnode_exit(lua_State *state) {
  qactor_t *actor;

  actor = qlua_get_actor(state);
  if (actor == NULL) {
    return 0;
  }
  actor->active = 0;
  qactor_destroy(actor);

  return 0;
}
