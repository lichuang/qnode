/*
 * See Copyright Notice in qnode.h
 */

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "qnet.h"

static void net_error(const char *fmt, ...) {
}

static int create_socket() {
  int fd, on = 1;
  if ((fd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
    net_error("creating socket: %s", strerror(errno));
    return -1;
  }

  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
    net_error("setsockopt SO_REUSEADDR: %s", strerror(errno));
    return -1;
  }
  return fd;
}

static int net_listen(int fd, struct sockaddr *sa, socklen_t len) {
  if (bind(fd, sa, len) < 0) {
    //net_error(err, "bind: %s", strerror(errno));
    close(fd);
    return -1;
  }

  if (listen(fd, 511) == -1) {
    net_error("listen: %s", strerror(errno));
    close(fd);
    return -1;
  }

  return 0;
}

int qnode_net_create_listener(int port, char *bindaddr) {
  int fd;
  struct sockaddr_in sa;

  if ((fd = create_socket()) < 0) {
    return -1;
  }
  memset(&sa,0,sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  sa.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bindaddr && inet_aton(bindaddr, &sa.sin_addr) == 0) {
    net_error("invalid bind address");
    close(fd);
    return -1;
  }
  if (net_listen(fd,(struct sockaddr*)&sa, sizeof(sa)) < 0) {
    close(fd);
    return -1;
  }
  return fd;
}

