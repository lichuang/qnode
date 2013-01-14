/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qactor.h"
#include "qassert.h"
#include "qconnection.h"
#include "qdefines.h"
#include "qengine.h"
#include "qluacapi.h"
#include "qluautil.h"
#include "qlog.h"
#include "qmsg.h"
#include "qnet.h"
#include "qstring.h"
#include "qserver.h"
#include "qthread.h"

static qactor_t* get_actor(lua_State *state) {
  lua_getglobal(state, "qnode");
  return (qactor_t*)lua_touserdata(state, -1);
}

/*
 * spawn an actor, return the actor ID
 * */
static int qnode_spawn(lua_State *state) {
  qactor_t *actor = get_actor(state);
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
  qactor_t *srqnode = get_actor(state);
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

static int qnode_tcp_listen(lua_State *state) {
  qactor_t *actor = get_actor(state);
  qassert(actor);
  qassert(actor->listen_fd == 0);
  //const char *addr = lua_tostring(state, 1);
  const char *addr = "0.0.0.0";
  int port = (int)lua_tonumber(state, 1);
  if (lua_type(state, 2) != LUA_TFUNCTION) {
    qerror("invalid listener on :%d\n", addr, port);
    return 0;
  }
  if (actor->lua_ref[LISTENER] != -1) {
    qerror("listener exist on actor listen to %s:%d\n", addr, port);
    return 0;
  }
  /* push listen callback */
  lua_pushvalue(state, 2);
  actor->lua_ref[LISTENER] = luaL_ref(state, LUA_REGISTRYINDEX);
  if (actor->lua_ref[LISTENER] == LUA_REFNIL) {
    qerror("ref listener on %s:%d error\n", addr, port);
    return 0;
  }
  /* pop listen callback */
  lua_pop(state, 1);
  int fd = qnet_tcp_listen(port, addr);
  if (fd < 0) {
    qerror("listen on %s:%d error\n", addr, port);
    return 0;
  }
  qthread_t *thread = g_server->threads[actor->tid];
  qengine_t *engine = thread->engine;
  if (qengine_add_event(engine, fd, QEVENT_READ, qactor_accept, actor) < 0) {
    qerror("add event on %s:%d error\n", addr, port);
    return 0;
  }
  actor->listen_fd = fd;
  lua_pushvalue(state, -3);

  return 0;
}

static int qnode_tcp_accept(lua_State *state) {
  qactor_t *actor = get_actor(state);
  qconnection_t *connection = qactor_get_connection(actor);
  lua_pushlightuserdata(actor->state, connection);
  return 1;
}

static int qnode_tcp_recv(lua_State *state) {
  UNUSED(state);
  return 0;
}

static int qnode_tcp_send(lua_State *state) {
  UNUSED(state);
  return 0;
}

static luaL_Reg apis[] = {
  {"qnode_spawn",       qnode_spawn},
  {"qnode_send",        qnode_send},
  {"qnode_tcp_listen",  qnode_tcp_listen},
  {"qnode_tcp_accept",  qnode_tcp_accept},
  {"qnode_tcp_recv",    qnode_tcp_recv},
  {"qnode_tcp_send",    qnode_tcp_send},
  {NULL, NULL},
};

void qluac_register(lua_State *state, struct qactor_t *actor) {
  int i;
  for (i = 0; apis[i].name != NULL; ++i) {
    const char *name = apis[i].name;
    lua_CFunction func = apis[i].func;
    lua_register(state, name, func);
  }
  lua_pushlightuserdata(state, actor);
  lua_setglobal(state, "qnode");
}
