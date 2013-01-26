/*
 * See Copyright Notice in qnode.h
 */

#include <errno.h>
#include "qactor.h"
#include "qassert.h"
#include "qdefines.h"
#include "qdescriptor.h"
#include "qengine.h"
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
    lua_pushfstring(state, "listen on %s:%d error", addr, port);
    return 2;
  }
  qdescriptor_t *desc = qdescriptor_new(fd, QDESCRIPTOR_TCP, actor);
  desc->data.tcp.inet.state = QINET_STATE_LISTENING;

  init_tcp_listen_params(actor);  
  lua_pushlightuserdata(state, desc);
  //qdict_copy_lua_table(actor->listen_params, state, 3);
  return 1;
}

static void node_accept(int fd, int flags, void *data) {
  UNUSED(fd);
  UNUSED(flags);
  struct sockaddr remote;
  socklen_t n = sizeof(struct sockaddr);
  qdescriptor_t *desc = (qdescriptor_t *)data;
  //qtcp_descriptor_t *tcp = &(desc->data.tcp);
  qactor_t *actor = qdescriptor_get_actor(desc);
  lua_State *state = actor->state;
  qinfo("add a socket....");
  int sock = qnet_tcp_accept(desc->fd, &remote, &n);
  if (sock == -1) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket closed");
    lua_resume(state, 2);
    return;
  }
  lua_pushnumber(state, sock);
  lua_resume(state, 1);
  return;
}

static int qnode_tcp_accept(lua_State *state) {
  struct sockaddr remote;
  socklen_t n = sizeof(struct sockaddr);
  qactor_t *actor = qlua_get_actor(state);
  qdescriptor_t *desc = (qdescriptor_t*)lua_touserdata(state, 1);
  qtcp_descriptor_t *tcp = &(desc->data.tcp);
  int timeout = (int)lua_tonumber(state, 2);
  if (tcp->inet.state != QINET_STATE_LISTENING &&
      tcp->inet.state != QINET_STATE_ACCEPTING) {
    lua_pushnil(state);
    lua_pushfstring(state, "socket state %d error", tcp->inet.state);
    return 2;
  }
  int fd = qnet_tcp_accept(desc->fd, &remote, &n);
  if (fd == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      qengine_t *engine = qactor_get_engine(actor);
      qengine_add_event(engine, desc->fd, QEVENT_READ, node_accept, desc);
      tcp->inet.state = QINET_STATE_ACCEPTING;
      return lua_yield(state, 0); 
    }

    lua_pushnil(state);
    lua_pushliteral(state, "socket closed");
    return 2;
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
