/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QNET_H__
#define __QNET_H__

#include <sys/types.h>
#include <sys/socket.h>

struct qtcp_descriptor_t;

int qnet_tcp_listen(int port, const char *bindaddr);
int qnet_tcp_accept(int listen_fd,  struct sockaddr *addr, socklen_t *addrlen);

int qnet_tcp_recv(struct qtcp_descriptor_t *desc, int request_len);

#endif  /* __QNET_H__ */
