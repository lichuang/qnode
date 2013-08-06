/*
 * See Copyright Notice in qnode.h
 */

#include <errno.h>
#include <arpa/inet.h>
#include "qactor.h"
#include "qassert.h"
#include "qdefines.h"
#include "qengine.h"
#include "qdict.h"
#include "qlog.h"
#include "qluautil.h"
#include "qnet.h"
#include "qserver.h"
#include "qsocket.h"
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
  qsocket_t   *socket;

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
  socket = qsocket_new(fd, actor);
  socket->state = QINET_STATE_LISTENING;
  socket->accept = 1;

  init_tcp_listen_params(actor);  
  lua_pushlightuserdata(state, socket);
  //qdict_copy_lua_table(actor->listen_params, state, 3);
  return 1;
}

static int
new_tcp_socket(int fd, lua_State *state, qactor_t *actor,
                   struct sockaddr_in *remote) {
  qsocket_t *socket;

  socket = qsocket_new(fd, actor);
  if (socket == NULL) {
    lua_pushnil(state);
    lua_pushliteral(state, "create socket error");
    qlua_resume(state, 2);
    return -1;
  }

  socket->state = QINET_STATE_CONNECTED;
  memcpy(&(socket->remote), remote, sizeof(*remote));
  inet_ntop(AF_INET, &remote->sin_addr,
            socket->addr, sizeof(socket->addr));
  socket->port = ntohs(remote->sin_port);
  snprintf(socket->peer, sizeof(socket->peer),
           "%s:%d:%d", socket->addr, socket->port, fd);
  qinfo("accept connection from %s", socket->peer);

  lua_pushlightuserdata(state, socket);
  return 0;
}

static void
socket_accept(int fd, int flags, void *data) {
  int                 sock;
  struct sockaddr_in  remote;
  socklen_t           n;
  qsocket_t          *socket;
  lua_State          *state;
  qengine_t          *engine;
  qactor_t           *actor;

  UNUSED(fd);
  UNUSED(flags);

  n = sizeof(remote);

  socket = (qsocket_t*)data;
  qassert(socket->state == QINET_STATE_ACCEPTING);
  actor = qactor_get(socket->aid);
  if (actor == NULL) {
    return;
  }
  actor->waiting_netio = 0;
  state = actor->state;
  sock = qnet_tcp_accept(socket->fd, (struct sockaddr*)&remote, &n);
  if (sock == -1) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket closed");
    qlua_resume(state, 2);
    return;
  }
  /* restore the listen fd state */
  engine = qactor_get_engine(actor->aid);
  qengine_del_event(engine, socket->fd, QEVENT_READ);
  socket->state = QINET_STATE_LISTENING;
  
  if (new_tcp_socket(sock, state, actor, &remote) < 0) {
    return;
  }
  qlua_resume(state, 1);
}

static int
tcp_accept(lua_State *state) {
  int                 fd;
  int                 timeout;
  struct sockaddr_in  remote;
  socklen_t           n;
  qactor_t           *actor;
  qsocket_t          *socket;

  socket = (qsocket_t*)lua_touserdata(state, 1);
  n = sizeof(remote);
  actor = qlua_get_actor(state);
  timeout = (int)lua_tonumber(state, 2);
  UNUSED(timeout);

  if (socket == NULL) {
    lua_pushnil(state);
    lua_pushfstring(state, "socket nil");
    return 2;
  }

  if (socket->state != QINET_STATE_LISTENING &&
      socket->state != QINET_STATE_ACCEPTING) {
    lua_pushnil(state);
    lua_pushfstring(state, "socket state %d error", socket->state);
    return 2;
  }

  fd = qnet_tcp_accept(socket->fd, (struct sockaddr*)&remote, &n);
  if (fd == -1) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket closed");
    return 2;
  }
  if (fd == 0) {
    qengine_t *engine = qactor_get_engine(actor->aid);
    qengine_add_event(engine, socket->fd, QEVENT_READ,
                      socket_accept, socket);
    socket->state = QINET_STATE_ACCEPTING;
    actor->waiting_netio = 1;
    return lua_yield(state, 0); 
  }

  return new_tcp_socket(fd, state, actor, &remote);
}

static void
socket_recv(int fd, int flags, void *data) {
  int                nret;
  qsocket_t         *socket;
  qactor_t          *actor;
  lua_State         *state;
  qengine_t         *engine;

  UNUSED(fd);
  UNUSED(flags);

  socket = (qsocket_t *)data;
  if (socket->state != QINET_STATE_CONNECTED) {
    return;
  }

  actor = qactor_get(socket->aid);
  if (actor == NULL) {
    return;
  }
  actor->waiting_netio = 0;
  state = actor->state;
  nret = qnet_tcp_recv(socket);
  if (nret < 0) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket closed");
    qlua_resume(state, 2);
    return;
  }
  engine = qactor_get_engine(actor->aid);
  qengine_del_event(engine, socket->fd, QEVENT_READ);
  lua_pushlightuserdata(state, socket->in);
  qlua_resume(state, 1);
}

static int
tcp_recv(lua_State *state) {
  int                 nret;
  qsocket_t          *socket;
  qactor_t           *actor;
  qengine_t          *engine;

  socket = (qsocket_t*)lua_touserdata(state, 1);
  if (socket->state != QINET_STATE_CONNECTED) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket closed");
    return 2;
  }

  nret = qnet_tcp_recv(socket);
  if (nret < 0) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket closed");
    return 2;
  }
  if (nret == 0) {
    actor = qlua_get_actor(state);
    engine = qactor_get_engine(actor->aid);
    qengine_add_event(engine, socket->fd,
                      QEVENT_READ, socket_recv, socket);
    actor->waiting_netio = 1;
    return lua_yield(state, 0); 
  }
  lua_pushlightuserdata(state, socket->in);
  return 1;
}

static void
socket_send(int fd, int flags, void *data) {
  int                 nret;
  qsocket_t          *socket;
  qactor_t           *actor;
  qengine_t          *engine;
  lua_State          *state;

  UNUSED(fd);
  UNUSED(flags);

  socket = (qsocket_t *)data;
  if (socket->state != QINET_STATE_CONNECTED) {
    return;
  }

  actor = qactor_get(socket->aid);
  if (actor == NULL) {
    return;
  }
  actor->waiting_netio = 0;
  state = actor->state;
  nret = qnet_tcp_send(socket);
  if (nret < 0) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket closed");
    qlua_resume(state, 2);
    return;
  }

  engine = qactor_get_engine(actor->aid);
  qengine_del_event(engine, socket->fd, QEVENT_WRITE);
  socket->in->end = 0;
  qlua_resume(state, 1);
}

static int
tcp_send(lua_State *state) {
  int                 nret;
  qsocket_t          *socket;
  qactor_t           *actor;
  qengine_t          *engine;

  socket = (qsocket_t*)lua_touserdata(state, 1);
  if (socket->state != QINET_STATE_CONNECTED) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket closed");
    return 2;
  }

  nret = qnet_tcp_send(socket);
  if (nret < 0) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket closed");
    return 2;
  }
  if (nret == 0) {
    actor = qlua_get_actor(state);
    engine = qactor_get_engine(actor->aid);
    qengine_add_event(engine, socket->fd, QEVENT_WRITE,
                      socket_send, socket);
    actor->waiting_netio = 1;
    return lua_yield(state, 0); 
  }

  lua_pushnumber(state, nret);
  return 1;
}

static int
tcp_buffer(lua_State *state, int in) {
  qsocket_t     *socket;

  socket = (qsocket_t*)lua_touserdata(state, 1);
  if (socket->state != QINET_STATE_CONNECTED) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket closed");
    return 2;
  }

  if (in) {
    lua_pushlightuserdata(state, socket->in);
  } else {
    lua_pushlightuserdata(state, socket->out);
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
  {"qtcp_in",        tcp_inbuf},
  {"qtcp_out",       tcp_outbuf},
  {NULL, NULL},
};
