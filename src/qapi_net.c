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

static void init_tcp_listen_params(qactor_t *actor);
static int  tcp_buffer(lua_State *state, int in);
static qsocket_t*  new_tcp_socket(int fd, lua_State *state,
                                  qactor_t *actor,
                                  struct sockaddr_in *remote);
static void socket_accept(int fd, int flags, void *data);
static void socket_connect(int fd, int flags, void *data);
static void socket_recv(int fd, int flags, void *data);
static void socket_send(int fd, int flags, void *data);
static int  tcp_buffer(lua_State *state, int in);

static int  qltcp_listen(lua_State *state);
static int  qltcp_accept(lua_State *state);
static int  qltcp_connect(lua_State *state);
static int  qltcp_recv(lua_State *state);
static int  qltcp_send(lua_State *state);
static int  qltcp_inbuf(lua_State *state);
static int  qltcp_outbuf(lua_State *state);

luaL_Reg net_apis[] = {
  {"qltcp_listen",    qltcp_listen},
  {"qltcp_accept",    qltcp_accept},
  {"qltcp_connect",   qltcp_connect},
  {"qltcp_recv",      qltcp_recv},
  {"qltcp_send",      qltcp_send},
  {"qltcp_inbuf",     qltcp_inbuf},
  {"qltcp_outbuf",    qltcp_outbuf},
  {NULL, NULL},
};

static void
init_tcp_listen_params(qactor_t *actor) {
  actor->listen_params = qdict_new(5);

  qdict_set_strnum(actor->listen_params, "packet", 0);
}

static int
getsockerr(int fd) {
  int err = 0;
  socklen_t len = sizeof(err);
  if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len) == -1) {
    err = errno;
  }
  return err;
}

static int
qltcp_listen(lua_State *state) {
  const char  *addr;
  qactor_t    *actor;
  int          port, fd, error;
  qsocket_t   *socket;

  actor = qlua_get_actor(state);

  port = (int)lua_tonumber(state, 1);
  if (port < 0) {
    lua_pushnil(state);
    lua_pushfstring(state, "wrong port %d", port);
    return 2;
  }

  addr = lua_tostring(state, 2);
  if (addr == NULL) {
    /* default 0.0.0.0 */
    addr = "0.0.0.0";
  }

  fd = qnet_tcp_listen(port, addr, &error);
  if (fd < 0) {
    lua_pushnil(state);
    lua_pushfstring(state, "listen on %s:%d error: %s", addr, port, strerror(error));
    return 2;
  }
  socket = qsocket_new(fd, actor);
  if (!socket) {
    qnet_close(fd);
    lua_pushnil(state);
    lua_pushfstring(state, "create socket on %s:%d error", addr, port);
    return 2;
  }

  socket->state = QINET_STATE_LISTENING;
  socket->accept = 1;
  socket->port   = port;
  strcpy(socket->addr, addr);
  snprintf(socket->peer, sizeof(socket->peer),
           "%s:%d:%d", socket->addr, socket->port, fd);

  init_tcp_listen_params(actor);  
  lua_pushlightuserdata(state, socket);
  //qdict_copy_lua_table(actor->listen_params, state, 3);
  return 1;
}

static void
init_socket_peer(qsocket_t *socket, struct sockaddr_in *remote) {
  memcpy(&(socket->remote), remote, sizeof(*remote));
  inet_ntop(AF_INET, &remote->sin_addr,
            socket->addr, sizeof(socket->addr));
  socket->port = ntohs(remote->sin_port);
  snprintf(socket->peer, sizeof(socket->peer),
           "%s:%d:%d", socket->addr, socket->port, socket->fd);
}

static void
socket_connected(qsocket_t *socket) {
  socket->state = QINET_STATE_CONNECTED;

  struct sockaddr_in remote;
  socklen_t addrlen = sizeof(remote);
  getpeername(socket->fd, (struct sockaddr*)&remote, &addrlen);
  init_socket_peer(socket, &remote);
}

static qsocket_t*
new_tcp_socket(int fd, lua_State *state, qactor_t *actor,
               struct sockaddr_in *remote) {
  qsocket_t *socket;

  socket = qsocket_new(fd, actor);
  if (socket == NULL) {
    return NULL;
  }

  socket->state = QINET_STATE_CONNECTED;
  init_socket_peer(socket, remote);
  qinfo("accept connection from %s", socket->peer);

  return socket;
}

static void
socket_accept(int fd, int flags, void *data) {
  int                 sock, error;
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
  sock = qnet_tcp_accept(socket->fd, (struct sockaddr*)&remote,
                         &n, &error);
  if (sock == -1) {
    lua_pushnil(state);
    lua_pushfstring(state, "%s accept error:%s",
                    socket->peer, strerror(error));
    qlua_resume(state, 2);
    return;
  }

  /* restore the listen fd state */
  engine = qactor_get_engine(actor->aid);
  qevent_del(&socket->event, QEVENT_READ);
  socket->state = QINET_STATE_LISTENING;
  
  socket = new_tcp_socket(sock, state, actor, &remote);
  if (!socket) {
    qnet_close(fd);
    lua_pushnil(state);
    lua_pushliteral(state, "create socket error");
    qlua_resume(state, 2);
    return;
  }
  lua_pushlightuserdata(state, socket);
  qlua_resume(state, 1);
  return;
}

static int
qltcp_accept(lua_State *state) {
  int                 fd, error;
  int                 timeout;
  struct sockaddr_in  remote;
  socklen_t           n;
  qactor_t           *actor;
  qsocket_t          *socket;
  qengine_t          *engine;

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

  if (!socket->accept) {
    lua_pushnil(state);
    lua_pushfstring(state, "not accept socket");
    return 2;
  }

  if (socket->state != QINET_STATE_LISTENING &&
      socket->state != QINET_STATE_ACCEPTING) {
    lua_pushnil(state);
    lua_pushfstring(state, "socket state %d error", socket->state);
    return 2;
  }

  fd = qnet_tcp_accept(socket->fd, (struct sockaddr*)&remote, &n, &error);
  if (fd == -1) {
    lua_pushnil(state);
    lua_pushfstring(state, "%s accept error:%s",
                    socket->peer, strerror(error));
    return 2;
  }
  if (fd == 0) {
    engine = qactor_get_engine(actor->aid);
    socket->state = QINET_STATE_ACCEPTING;
    socket->event.read = socket_accept;
    qevent_add(engine, &socket->event, QEVENT_READ);
    actor->waiting_netio = 1;
    return lua_yield(state, 0); 
  }

  socket = new_tcp_socket(fd, state, actor, &remote);
  if (!socket) {
    qnet_close(fd);
    lua_pushnil(state);
    lua_pushliteral(state, "create socket error");
    return 2;
  }

  lua_pushlightuserdata(state, socket);
  return 1;
}

static void
socket_connect(int fd, int flags, void *data) {
  int                 error;
  qsocket_t          *socket;
  lua_State          *state;
  qengine_t          *engine;
  qactor_t           *actor;

  UNUSED(flags);

  socket = (qsocket_t*)data;
  qevent_del(&socket->event, QEVENT_WRITE);
  qassert(socket->state == QINET_STATE_CONNECTING);

  actor = qactor_get(socket->aid);
  if (actor == NULL) {
    return;
  }
  actor->waiting_netio = 0;
  state = actor->state;
  engine = qactor_get_engine(actor->aid);

  error = getsockerr(fd);
  if (error != 0) {
    qsocket_free(socket);
    lua_pushnil(state);
    lua_pushstring(state, strerror(error));
    qlua_resume(state, 2);
    return;
  }

  socket_connected(socket);
  lua_pushlightuserdata(state, socket);
  qlua_resume(state, 1);
  return;
}

static int
qltcp_connect(lua_State *state) {
  const char  *addr;
  qactor_t    *actor;
  int          port, fd, error, ret;
  qsocket_t   *socket;
  qengine_t   *engine;

  actor = qlua_get_actor(state);

  addr = lua_tostring(state, 1);
  if (addr == NULL) {
    lua_pushnil(state);
    lua_pushfstring(state, "addr nil");
    return 2;
  }

  port = (int)lua_tonumber(state, 2);
  if (port < 0) {
    lua_pushnil(state);
    lua_pushfstring(state, "wrong port %d", port);
    return 2;
  }

  ret = qnet_tcp_connect(port, addr, &error, &fd);

  if (ret == QERROR) {
    lua_pushnil(state);
    lua_pushfstring(state, "connect to %s:%d error: %s",
                    addr, port, strerror(error));
    return 2;
  }

  socket = qsocket_new(fd, actor);
  if (!socket) {
    qnet_close(fd);
    lua_pushnil(state);
    lua_pushfstring(state, "create socket on %s:%d error", addr, port);
    return 2;
  }

  if (ret == QNONBLOCKING) {
    engine = qactor_get_engine(actor->aid);
    socket->state = QINET_STATE_CONNECTING;
    socket->event.write = socket_connect;
    qevent_add(engine, &socket->event, QEVENT_WRITE);
    actor->waiting_netio = 1;
    return lua_yield(state, 0); 
  }

  socket_connected(socket);
  lua_pushlightuserdata(state, socket);
  return 1;
}

static void
socket_recv(int fd, int flags, void *data) {
  int                nret, error;
  qsocket_t         *socket;
  qactor_t          *actor;
  lua_State         *state;
  qengine_t         *engine;

  UNUSED(fd);
  UNUSED(flags);

  socket = (qsocket_t *)data;
  if (!socket) {
    return;
  }

  actor = qactor_get(socket->aid);
  if (actor == NULL) {
    return;
  }

  state = actor->state;
  if (socket->state != QINET_STATE_CONNECTED) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket not in connected state");
    qlua_resume(state, 2);
    return;
  }

  actor->waiting_netio = 0;
  nret = qnet_tcp_recv(socket, &error);
  if (nret < 0) {
    lua_pushnil(state);
    lua_pushfstring(state, "recv from %s error:%s",
                    socket->peer, strerror(error));
    qlua_resume(state, 2);
    return;
  }
  engine = qactor_get_engine(actor->aid);
  qevent_del(&socket->event, QEVENT_READ);
  lua_pushnumber(state, nret);
  qlua_resume(state, 1);
}

static int
qltcp_recv(lua_State *state) {
  int                 nret, error;
  qsocket_t          *socket;
  qactor_t           *actor;
  qengine_t          *engine;

  socket = (qsocket_t*)lua_touserdata(state, 1);
  if (!socket) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket nil");
    return 2;
  }
  if (socket->state != QINET_STATE_CONNECTED) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket not in connected state");
    return 2;
  }

  nret = qnet_tcp_recv(socket, &error);
  if (nret < 0) {
    qevent_del(&socket->event, socket->event.events);
    qnet_close(socket->event.fd);
    lua_pushnil(state);
    lua_pushfstring(state, "recv from %s error:%s",
                    socket->peer, strerror(error));
    return 2;
  }
  if (nret == 0) {
    actor = qlua_get_actor(state);
    engine = qactor_get_engine(actor->aid);
    socket->event.read = socket_recv;
    qevent_add(engine, &socket->event, QEVENT_READ);
    actor->waiting_netio = 1;
    return lua_yield(state, 0); 
  }
  lua_pushnumber(state, nret);
  return 1;
}

static void
socket_send(int fd, int flags, void *data) {
  int                 nret, error;
  qsocket_t          *socket;
  qactor_t           *actor;
  qengine_t          *engine;
  lua_State          *state;

  UNUSED(fd);
  UNUSED(flags);

  socket = (qsocket_t *)data;
  if (!socket) {
    return;
  }

  actor = qactor_get(socket->aid);
  if (actor == NULL) {
    return;
  }

  state = actor->state;
  if (socket->state != QINET_STATE_CONNECTED) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket not in connected state");
    qlua_resume(state, 2);
    return;
  }

  actor->waiting_netio = 0;
  nret = qnet_tcp_send(socket, &error);
  if (nret < 0) {
    lua_pushnil(state);
    lua_pushfstring(state, "send to %s error:%s",
                    socket->peer, strerror(error));
    qlua_resume(state, 2);
    return;
  }

  engine = qactor_get_engine(actor->aid);
  qevent_del(&socket->event, QEVENT_WRITE);
  socket->in->end = 0;
  lua_pushnumber(state, nret);
  qlua_resume(state, 1);
}

static int
qltcp_send(lua_State *state) {
  int                 nret, error;
  qsocket_t          *socket;
  qactor_t           *actor;
  qengine_t          *engine;

  socket = (qsocket_t*)lua_touserdata(state, 1);
  if (socket->state != QINET_STATE_CONNECTED) {
    lua_pushnil(state);
    lua_pushliteral(state, "socket not in connected state");
    return 2;
  }

  nret = qnet_tcp_send(socket, &error);
  if (nret < 0) {
    qevent_del(&socket->event, socket->event.events);
    qnet_close(socket->event.fd);
    lua_pushnil(state);
    lua_pushfstring(state, "send to %s error:%s",
                    socket->peer, strerror(error));
    return 2;
  }
  if (nret == 0) {
    actor = qlua_get_actor(state);
    engine = qactor_get_engine(actor->aid);
    socket->event.write = socket_send;
    qevent_add(engine, &socket->event, QEVENT_WRITE);
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
qltcp_inbuf(lua_State *state) {
  return tcp_buffer(state, 1);
}

static int
qltcp_outbuf(lua_State *state) {
  return tcp_buffer(state, 0);
}
