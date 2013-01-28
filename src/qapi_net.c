/*
 * See Copyright Notice in qnode.h
 */

#include "qactor.h"
#include "qassert.h"
#include "qdefines.h"
#include "qdescriptor.h"
#include "qdict.h"
#include "qengine.h"
#include "qlog.h"
#include "qluautil.h"
#include "qnet.h"
#include "qserver.h"
#include "qstring.h"
#include "qthread.h"

static void init_tcp_listen_params(qactor_t *actor) {
  actor->listen_params = qdict_new(5);

  {
    qkey_t key;
    QKEY_STRING(key, "packet");
    qdict_val_t val;
    QVAL_NUMBER(val, 0);
    qdict_add(actor->listen_params, &key, &val);
  }
}

static int qnode_tcp_listen(lua_State *state) {
  qactor_t *actor = qlua_get_actor(state);
  qassert(actor);
  //const char *addr = lua_tostring(state, 1);
  const char *addr = "0.0.0.0";
  int port = (int)lua_tonumber(state, 1);
  int fd = qnet_tcp_listen(port, addr);
  if (fd < 0) {
    lua_pushnil(state);
    qstring_t str =  qstring_init();
    qstring_format(&str, "listen on %s:%d error", addr, port);
    lua_pushlstring(state, str.data, str.len);
    qerror(str.data);
    qstring_destroy(&str);
    return 2;
  }
  qdescriptor_t *desc = qdescriptor_new(fd, QDESCRIPTOR_TCP, actor);
  desc->data.tcp.inet.state = QINET_STATE_LISTENING;

  init_tcp_listen_params(actor);  
  lua_pushlightuserdata(state, desc);
  //qdict_copy_lua_table(actor->listen_params, state, 3);
  return 1;
}

static int qnode_tcp_accept(lua_State *state) {
  qactor_t *actor = qlua_get_actor(state);
  qdescriptor_t *desc = (qdescriptor_t*)lua_touserdata(state, 1);
  qtcp_descriptor_t *tcp = &(desc->data.tcp);
  int timeout = (int)lua_tonumber(state, 2);
  if (tcp->inet.state != QINET_STATE_LISTENING) {
    lua_pushnil(state);
    qstring_t str =  qstring_init();
    qstring_format(&str, "socket closed");
    lua_pushlstring(state, str.data, str.len);
    qerror(str.data);
    qstring_destroy(&str);
    return 2;
  }
  int fd = qnet_tcp_accept(desc->fd);
  if (fd == -1) {
    qerror("accept error");
    return lua_yield(state, 0); 
  }
  qdescriptor_t *accept_desc = qdescriptor_new(fd, QDESCRIPTOR_TCP, actor);
  desc->data.tcp.inet.state = QINET_STATE_CONNECTED;
  UNUSED(timeout);
  lua_pushlightuserdata(state, accept_desc);
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

luaL_Reg net_apis[] = {
  {"qnode_tcp_listen",  qnode_tcp_listen},
  {"qnode_tcp_accept",  qnode_tcp_accept},
  {"qnode_tcp_recv",    qnode_tcp_recv},
  {"qnode_tcp_send",    qnode_tcp_send},
  {NULL, NULL},
};
