/*
 * See Copyright Notice in qnode.h
 */

#include <errno.h>
#include <arpa/inet.h>
#include "qactor.h"
#include "qassert.h"
#include "qdefines.h"
#include "qdescriptor.h"
#include "qengine.h"
#include "qdict.h"
#include "qlog.h"
#include "qluautil.h"
#include "qnet.h"
#include "qserver.h"
#include "qstring.h"
#include "qworker.h"

static int tcp_buffer(lua_State *state, int in);

static void
init_tcp_listen_params(qactor_t *actor) {
  actor->listen_params = qdict_new(5);

  qdict_setnum(actor->listen_params, "packet", 0);
}

static int
tcp_listen(lua_State *state) {
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
  qdescriptor_t *desc = qdescriptor_new(fd, QDESCRIPTOR_TCP, actor);
  desc->data.tcp.inet.state = QINET_STATE_LISTENING;

  init_tcp_listen_params(actor);  
  lua_pushlightuserdata(state, desc);
  //qdict_copy_lua_table(actor->listen_params, state, 3);
  return 1;
}

static int
new_tcp_descriptor(int fd, lua_State *state, qactor_t *actor,
                   struct sockaddr_in *remote) {
  qdescriptor_t      *desc;
  qtcp_descriptor_t  *tcp;

  desc = qdescriptor_new(fd, QDESCRIPTOR_TCP, actor);
  if (desc == NULL) {
    lua_pushnil(state);
    lua_pushliteral(state, "create descriptor error");
    lua_resume(state, 2);
    return -1;
  }

  tcp = &(desc->data.tcp);
  tcp->inet.state = QINET_STATE_CONNECTED;
  memcpy(&(tcp->remote), remote, sizeof(*remote));
  inet_ntop(AF_INET, &remote->sin_addr, tcp->addr, sizeof(tcp->addr));
  tcp->port = ntohs(remote->sin_port);
  snprintf(tcp->peer, sizeof(tcp->peer),
           "%s:%d:%d", tcp->addr, tcp->port, fd);
  qinfo("accept connection from %s", tcp->peer);

  lua_pushlightuserdata(state, desc);
  return 0;
}

static void
socket_accept(int fd, int flags, void *data) {
  int                 sock;
  struct sockaddr_in  remote;
  socklen_t           n;
  qdescriptor_t      *desc;
  qtcp_descriptor_t  *tcp;
  lua_State          *state;
  qengine_t          *engine;
  qactor_t           *actor;

  UNUSED(fd);
  UNUSED(flags);

  n = sizeof(remote);
  desc = (qdescriptor_t *)data;
  tcp = &(desc->data.tcp);

  qassert(tcp->inet.state == QINET_STATE_ACCEPTING);
  actor = qactor_get(desc->aid);
  if (actor == NULL) {
    return;
  }
  actor->waiting_netio = 0;
  state = actor->state;
  sock = qnet_tcp_accept(desc->fd, (struct sockaddr*)&remote, &n);
  if (sock == -1) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket closed");
    lua_resume(state, 2);
    return;
  }
  /* restore the listen fd state */
  engine = qactor_get_engine(actor->aid);
  qengine_del_event(engine, desc->fd, QEVENT_READ);
  desc->data.tcp.inet.state = QINET_STATE_LISTENING;
  
  if (new_tcp_descriptor(sock, state, actor, &remote) < 0) {
    return;
  }
  lua_resume(state, 1);
}

static int
tcp_accept(lua_State *state) {
  int                 fd;
  int                 timeout;
  struct sockaddr_in  remote;
  socklen_t           n;
  qactor_t           *actor;
  qdescriptor_t      *desc;
  qtcp_descriptor_t  *tcp;

  n = sizeof(remote);
  actor = qlua_get_actor(state);
  desc = (qdescriptor_t*)lua_touserdata(state, 1);
  tcp = &(desc->data.tcp);
  timeout = (int)lua_tonumber(state, 2);
  UNUSED(timeout);

  if (desc == NULL) {
    lua_pushnil(state);
    lua_pushfstring(state, "socket descriptor nil");
    return 2;
  }

  if (tcp->inet.state != QINET_STATE_LISTENING &&
    tcp->inet.state != QINET_STATE_ACCEPTING) {
    lua_pushnil(state);
    lua_pushfstring(state, "socket state %d error", tcp->inet.state);
    return 2;
  }

  fd = qnet_tcp_accept(desc->fd, (struct sockaddr*)&remote, &n);
  if (fd == -1) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket closed");
    return 2;
  }
  if (fd == 0) {
    qengine_t *engine = qactor_get_engine(actor->aid);
    qengine_add_event(engine, desc->fd, QEVENT_READ, socket_accept, desc);
    tcp->inet.state = QINET_STATE_ACCEPTING;
    actor->waiting_netio = 1;
    return lua_yield(state, 0); 
  }

  return new_tcp_descriptor(fd, state, actor, &remote);
}

static void
socket_recv(int fd, int flags, void *data) {
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

  actor = qactor_get(desc->aid);
  if (actor == NULL) {
    return;
  }
  actor->waiting_netio = 0;
  state = actor->state;
  nret = qnet_tcp_recv(desc);
  if (nret < 0) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket closed");
    lua_resume(state, 2);
    return;
  }
  engine = qactor_get_engine(actor->aid);
  qengine_del_event(engine, desc->fd, QEVENT_READ);
  lua_pushlightuserdata(state, desc->data.tcp.inbuf);
  lua_resume(state, 1);
}

static int
tcp_recv(lua_State *state) {
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

  nret = qnet_tcp_recv(desc);
  if (nret < 0) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket closed");
    return 2;
  }
  if (nret == 0) {
    actor = qlua_get_actor(state);
    engine = qactor_get_engine(actor->aid);
    qengine_add_event(engine, desc->fd, QEVENT_READ, socket_recv, desc);
    actor->waiting_netio = 1;
    return lua_yield(state, 0); 
  }
  lua_pushlightuserdata(state, desc->data.tcp.inbuf);
  return 1;
}

static void
socket_send(int fd, int flags, void *data) {
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

  actor = qactor_get(desc->aid);
  if (actor == NULL) {
    return;
  }
  actor->waiting_netio = 0;
  state = actor->state;
  nret = qnet_tcp_send(desc);
  if (nret < 0) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket closed");
    lua_resume(state, 2);
    return;
  }

  engine = qactor_get_engine(actor->aid);
  qengine_del_event(engine, desc->fd, QEVENT_WRITE);
  desc->data.tcp.inbuf->end = 0;
  lua_resume(state, 1);
}

static int
tcp_send(lua_State *state) {
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
    engine = qactor_get_engine(actor->aid);
    qengine_add_event(engine, desc->fd, QEVENT_WRITE, socket_send, desc);
    actor->waiting_netio = 1;
    return lua_yield(state, 0); 
  }

  lua_pushnumber(state, nret);
  return 1;
}

static int
tcp_buffer(lua_State *state, int in) {
  qdescriptor_t     *desc;
  qtcp_descriptor_t *tcp;

  desc = (qdescriptor_t*)lua_touserdata(state, 1);
  tcp = &(desc->data.tcp);
  if (tcp->inet.state != QINET_STATE_CONNECTED) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket closed");
    return 2;
  }

  if (in) {
    lua_pushlightuserdata(state, tcp->inbuf);
  } else {
    lua_pushlightuserdata(state, tcp->outbuf);
  }
  return 1;
}

static int
tcp_inbuf(lua_State *state) {
  return tcp_buffer(state, 1);
}

static int
tcp_outbuf(lua_State *state) {
  return tcp_buffer(state, 0);
}

luaL_Reg net_apis[] = {
  {"qtcp_listen",    tcp_listen},
  {"qtcp_accept",    tcp_accept},
  {"qtcp_recv",      tcp_recv},
  {"qtcp_send",      tcp_send},
  {"qtcp_inbuf",     tcp_inbuf},
  {"qtcp_outbuf",    tcp_outbuf},
  {NULL, NULL},
};
