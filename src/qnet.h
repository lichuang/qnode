/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QNET_H__
#define __QNET_H__

int qnet_tcp_listen(int port, const char *bindaddr);
int qnet_tcp_accept(int fd);

#endif  /* __QNET_H__ */
