/*
 * See Copyright Notice in qnode.h
 */

#include "qassert.h"
#include "qconnection.h"
#include "qmalloc.h"
#include "qserver.h"

qconnection_t* qconnection_get(int fd) {
  qassert(fd < QID_MAX);
  qconnection_t *connection = g_server->connections[fd];
  if (connection) {
    qassert(connection);
    qassert(connection->aid == -1);
    qassert(connection->fd == fd);
  } else {
    connection = qalloc_type(qconnection_t);
    connection->aid = -1;
    connection->fd = fd;
    g_server->connections[fd] = connection;
    qlist_entry_init(&connection->entry);
  }
  return connection;
}
