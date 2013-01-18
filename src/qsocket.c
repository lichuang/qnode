/*
 * See Copyright Notice in qnode.h
 */

#include "qassert.h"
#include "qmalloc.h"
#include "qserver.h"
#include "qsocket.h"

qsocket_t* qsocket_get(int fd) {
  qassert(fd < QID_MAX);
  qsocket_t *socket = g_server->sockets[fd];
  if (socket) {
    qassert(socket);
    qassert(socket->aid == -1);
    qassert(socket->fd == fd);
  } else {
    socket = qalloc_type(qsocket_t);
    socket->aid = -1;
    socket->fd = fd;
    g_server->sockets[fd] = socket;
    qlist_entry_init(&socket->entry);
  }
  return socket;
}
