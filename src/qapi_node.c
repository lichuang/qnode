/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qactor.h"
#include "qassert.h"
#include "qdefines.h"
#include "qdescriptor.h"
#include "qdict.h"
#include "qluautil.h"
#include "qlog.h"
#include "qmsg.h"
#include "qmutex.h"
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
  qstring_t string = qstring_init();
  qstring_assign(&string, mod);
  qstring_append(&string, ".lua");
  if (qlua_threadloadfile(new_state, string.data) != 0) {
    lua_pushnil(state);
    lua_pushfstring(state, "load file %s error", string.data);
    return 2;
  }
  qstring_destroy(&string);

  /* copy args table */
  qlua_copy_state_table(state, new_state, 3);

  lua_getglobal(new_state, mod);
  lua_getfield(new_state, -1, fun);
  /* push the args table */
  lua_pushvalue(new_state, 1);
  int id = qactor_spawn(actor, new_state);
  if (id == -1) {
    lua_pushnil(state);
    lua_pushliteral(state, "spawn error");
    return 2;
  }
  lua_pushnumber(state, id);
  return 1;
}

static int qnode_send(lua_State *state) {
  qactor_t *src_actor = qlua_get_actor(state);
  qid_t id = (qid_t)lua_tonumber(state, 1);
  qactor_t *dst_actor = qserver_get_actor(id);
  if (dst_actor == NULL) {
    lua_pushnil(state);
    lua_pushfstring(state, "dst actor %d not found", id);
    return 2;
  }
  /* copy args table */
  qactor_msg_t *actor_msg = qactor_msg_new();
  qlua_copy_table(state, 2, &actor_msg->arg_dict);
  actor_msg->src = src_actor->aid;
  actor_msg->dst = dst_actor->aid;

  qmsg_t *msg = qmsg_new(src_actor->tid, dst_actor->tid);
  qmsg_init_tsend(msg, actor_msg);
  qmsg_send(msg);
  lua_pushnumber(state, 0);
  return 1;
}

static int qnode_recv(lua_State *state) {
  qactor_t *actor = qlua_get_actor(state);

  /* 
   * if msg list is empty, YIELD the Lua coroutine
   */
  if (qlist_empty(&(actor->msg_list))) {
    actor->waiting_msg = 1;
    return lua_yield(state, 0); 
  }
  qactor_msg_t *msg = qlist_entry(actor->msg_list.next, qactor_msg_t, entry); 
  lua_createtable(state, 0, 0);
  lua_pushlightuserdata(state, msg);
  return 1;
}

static int qnode_attach(lua_State *state) {
  qdescriptor_t *desc = (qdescriptor_t*)lua_touserdata(state, 1);
  qactor_t *old_actor = qdescriptor_get_actor(desc);
  qactor_t *actor = qlua_get_actor(state);
  if (old_actor->aid == actor->aid) {
    return 0;
  }
  /*
   * detach from old actor
   * */
  qspinlock_lock(&(old_actor->desc_list_lock));
  qlist_del_init(&(desc->entry));
  qspinlock_unlock(&(old_actor->desc_list_lock));
  /*
   * attach to new actor
   * */
  qspinlock_lock(&(actor->desc_list_lock));
  desc->aid = actor->aid;
  qlist_add_tail(&desc->entry, &actor->desc_list);
  qspinlock_unlock(&(actor->desc_list_lock));
  return 0;
}

luaL_Reg node_apis[] = {
  {"qnode_spawn",       qnode_spawn},
  {"qnode_send",        qnode_send},
  {"qnode_recv",        qnode_recv},
  {"qnode_attach",      qnode_attach},
  {NULL, NULL},
};
