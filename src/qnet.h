/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QNET_H__
#define __QNET_H__

#include <sys/types.h>
#include <sys/socket.h>
#include "qcore.h"

int qnet_tcp_listen(int port, const char *bindaddr);
int qnet_tcp_accept(int listen_fd,  struct sockaddr *addr, socklen_t *addrlen);

int qnet_tcp_recv(qsocket_t *socket);
int qnet_tcp_send(qsocket_t *socket);

#endif  /* __QNET_H__ */
