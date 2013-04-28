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
  actor->listen_params = qdict_new(actor->pool, 5);

  {
    qkey_t key;
    qval_t val;

    QKEY_STRING(key, "packet", actor->pool);
    QVAL_NUMBER(val, 0);
    qdict_add(actor->listen_params, &key, &val);
  }
}

static int qnode_tcp_listen(lua_State *state) {
  const char  *addr;
  qactor_t    *actor;
  int          port, fd;

  addr = "0.0.0.0";
  actor = qlua_get_actor(state);
  qassert(actor);
  //const char *addr = lua_tostring(state, 1);
  port = (int)lua_tonumber(state, 1);
  fd = qnet_tcp_listen(port, addr);
  if (fd < 0) {
    lua_pushnil(state);
    lua_pushfstring(state, "listen on %s:%d error", addr, port);
    return 2;
  }
  qdescriptor_t *desc = qdescriptor_new(actor->pool, fd, QDESCRIPTOR_TCP, actor);
  desc->data.tcp.inet.state = QINET_STATE_LISTENING;

  init_tcp_listen_params(actor);  
  lua_pushlightuserdata(state, desc);
  //qdict_copy_lua_table(actor->listen_params, state, 3);
  return 1;
}

static void socket_accept(int fd, int flags, void *data) {
  int                 sock;
  struct sockaddr     remote;
  socklen_t           n;
  qdescriptor_t      *desc;
  qtcp_descriptor_t  *tcp;
  qactor_t           *actor;
  lua_State          *state;
  qengine_t          *engine;
  qdescriptor_t      *accept_desc;

  UNUSED(fd);
  UNUSED(flags);

  n = sizeof(struct sockaddr);
  desc = (qdescriptor_t *)data;
  tcp = &(desc->data.tcp);

  qassert(tcp->inet.state == QINET_STATE_ACCEPTING);
  actor = qdescriptor_get_actor(desc);
  state = actor->state;
  qinfo("add a socket....");
  sock = qnet_tcp_accept(desc->fd, &remote, &n);
  if (sock == -1) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket closed");
    lua_resume(state, 2);
    return;
  }
  /* restore the listen fd state */
  engine = qactor_get_engine(actor);
  qengine_del_event(engine, desc->fd, QEVENT_READ);
  desc->data.tcp.inet.state = QINET_STATE_LISTENING;

  accept_desc = qdescriptor_new(actor->pool, sock, QDESCRIPTOR_TCP, actor);
  accept_desc->data.tcp.inet.state = QINET_STATE_CONNECTED;
  lua_pushlightuserdata(state, accept_desc);
  lua_resume(state, 1);
  return;
}

static int qnode_tcp_accept(lua_State *state) {
  int                 fd;
  int                 timeout;
  struct sockaddr     remote;
  socklen_t           n;
  qactor_t           *actor;
  qdescriptor_t      *desc;
  qtcp_descriptor_t  *tcp;

  n = sizeof(struct sockaddr);
  actor = qlua_get_actor(state);
  desc = (qdescriptor_t*)lua_touserdata(state, 1);
  tcp = &(desc->data.tcp);
  timeout = (int)lua_tonumber(state, 2);
  UNUSED(timeout);

  if (tcp->inet.state != QINET_STATE_LISTENING &&
    tcp->inet.state != QINET_STATE_ACCEPTING) {
    lua_pushnil(state);
    lua_pushfstring(state, "socket state %d error", tcp->inet.state);
    return 2;
  }

  fd = qnet_tcp_accept(desc->fd, &remote, &n);
  if (fd == -1) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket closed");
    return 2;
  }
  if (fd == 0) {
    qengine_t *engine = qactor_get_engine(actor);
    qengine_add_event(engine, desc->fd, QEVENT_READ, socket_accept, desc);
    tcp->inet.state = QINET_STATE_ACCEPTING;
    return lua_yield(state, 0); 
  }
  qdescriptor_t *accept_desc = qdescriptor_new(actor->pool, fd, QDESCRIPTOR_TCP, actor);
  accept_desc->data.tcp.inet.state = QINET_STATE_CONNECTED;
  lua_pushlightuserdata(state, accept_desc);
  return 1;
}

static void socket_recv(int fd, int flags, void *data) {
  int                nret;
  qdescriptor_t     *desc;
  qtcp_descriptor_t *tcp;
  qactor_t          *actor;
  lua_State         *state;
  qengine_t         *engine;

  UNUSED(fd);
  UNUSED(flags);

  desc = (qdescriptor_t *)data;
  tcp = &(desc->data.tcp);
  if (tcp->inet.state != QINET_STATE_CONNECTED) {
    return;
  }

  actor = qdescriptor_get_actor(desc);
  state = actor->state;
  nret = qnet_tcp_recv(desc, 0);
  if (nret < 0) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket closed");
    lua_resume(state, 2);
    return;
  }
  engine = qactor_get_engine(actor);
  qengine_del_event(engine, desc->fd, QEVENT_READ);
  desc->data.tcp.buffer.pos = 0;
  lua_pushlightuserdata(state, &(desc->data.tcp.buffer));
  lua_resume(state, 1);
}

static int qnode_tcp_recv(lua_State *state) {
  int                 nret;
  qdescriptor_t      *desc;
  qtcp_descriptor_t  *tcp;
  qactor_t           *actor;
  qengine_t          *engine;

  desc = (qdescriptor_t*)lua_touserdata(state, 1);
  tcp = &(desc->data.tcp);
  if (tcp->inet.state != QINET_STATE_CONNECTED) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket closed");
    return 2;
  }

  nret = qnet_tcp_recv(desc, 0);
  if (nret < 0) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket closed");
    return 2;
  }
  if (nret == 0) {
    actor = qlua_get_actor(state);
    engine = qactor_get_engine(actor);
    qengine_add_event(engine, desc->fd, QEVENT_READ, socket_recv, desc);
    return lua_yield(state, 0); 
  }
  desc->data.tcp.buffer.pos = 0;
  lua_pushlightuserdata(state, &(desc->data.tcp.buffer));
  return 1;
}

static void socket_send(int fd, int flags, void *data) {
  int                 nret;
  qdescriptor_t      *desc;
  qtcp_descriptor_t  *tcp;
  qactor_t           *actor;
  qengine_t          *engine;
  lua_State          *state;

  UNUSED(fd);
  UNUSED(flags);

  desc = (qdescriptor_t *)data;
  tcp = &(desc->data.tcp);
  if (tcp->inet.state != QINET_STATE_CONNECTED) {
    return;
  }

  actor = qdescriptor_get_actor(desc);
  state = actor->state;
  nret = qnet_tcp_send(desc);
  if (nret < 0) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket closed");
    lua_resume(state, 2);
    return;
  }

  engine = qactor_get_engine(actor);
  qengine_del_event(engine, desc->fd, QEVENT_WRITE);
  desc->data.tcp.buffer.pos = 0;
  lua_resume(state, 1);
}

static int qnode_tcp_send(lua_State *state) {
  int                 nret;
  qdescriptor_t      *desc;
  qtcp_descriptor_t  *tcp;
  qactor_t           *actor;
  qengine_t          *engine;

  desc = (qdescriptor_t*)lua_touserdata(state, 1);
  tcp = &(desc->data.tcp);
  if (tcp->inet.state != QINET_STATE_CONNECTED) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket closed");
    return 2;
  }

  nret = qnet_tcp_send(desc);
  if (nret < 0) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket closed");
    return 2;
  }
  if (nret == 0) {
    actor = qlua_get_actor(state);
    engine = qactor_get_engine(actor);
    qengine_add_event(engine, desc->fd, QEVENT_WRITE, socket_send, desc);
    return lua_yield(state, 0); 
  }

  lua_pushnumber(state, 0);
  return 1;
}

static int qnode_tcp_buffer(lua_State *state) {
  qdescriptor_t     *desc;
  qtcp_descriptor_t *tcp;

  desc = (qdescriptor_t*)lua_touserdata(state, 1);
  tcp = &(desc->data.tcp);
  if (tcp->inet.state != QINET_STATE_CONNECTED) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket closed");
    return 2;
  }

  lua_pushlightuserdata(state, &(tcp->buffer));
  return 1;
}

luaL_Reg net_apis[] = {
  {"qnode_tcp_listen",    qnode_tcp_listen},
  {"qnode_tcp_accept",    qnode_tcp_accept},
  {"qnode_tcp_recv",      qnode_tcp_recv},
  {"qnode_tcp_send",      qnode_tcp_send},
  {"qnode_tcp_buffer",    qnode_tcp_buffer},
  {NULL, NULL},
};
