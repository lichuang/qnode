/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QNET_H__
#define __QNET_H__

#include <sys/types.h>
#include <sys/socket.h>
#include "qcore.h"

int   qnet_tcp_listen(int port, const char *bindaddr, int *error);
int   qnet_tcp_accept(int listen_fd,  struct sockaddr *addr,
                      socklen_t *addrlen, int *error);

int   qnet_tcp_recv(qsocket_t *socket, int *error);
int   qnet_tcp_send(qsocket_t *socket, int *error);
void  qnet_close(int fd);

#endif  /* __QNET_H__ */
