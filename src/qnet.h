/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QNET_H__
#define __QNET_H__

#include <sys/types.h>
#include <sys/socket.h>

struct qdescriptor_t;
struct qtcp_descriptor_t;

int qnet_tcp_listen(int port, const char *bindaddr);
int qnet_tcp_accept(int listen_fd,  struct sockaddr *addr, socklen_t *addrlen);

int qnet_tcp_recv(struct qdescriptor_t *desc);
int qnet_tcp_send(struct qdescriptor_t *desc);

#endif  /* __QNET_H__ */
