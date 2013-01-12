/*
 * See Copyright Notice in qnode.h
 */

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "qassert.h"
#include "qlog.h"
#include "qnet.h"

static int create_socket() {
    int fd, on = 1;
    if ((fd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        qerror("creating socket: %s", strerror(errno));
        return -1;
    }

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
        qerror("setsockopt SO_REUSEADDR: %s", strerror(errno));
        return -1;
    }
    return fd;
}

static int net_listen(int fd, struct sockaddr *sa, socklen_t len) {
    if (bind(fd, sa, len) < 0) {
        qerror("bind error: %s", strerror(errno));
        close(fd);
        return -1;
    }

    if (listen(fd, 511) == -1) {
        qerror("listen: %s", strerror(errno));
        close(fd);
        return -1;
    }

    return 0;
}

int set_nonblocking(int fd) {
    int flags;

    if ((flags = fcntl(fd, F_GETFL)) == -1) {
        qerror("fcntl(F_GETFL): %s", strerror(errno));
        return -1;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        qerror("fcntl(F_SETFL,O_NONBLOCK): %s", strerror(errno));
        return -1;
    }
    return 0;
}

int qnet_tcp_listen(int port, const char *bindaddr) {
    int fd;
    struct sockaddr_in sa;

    if ((fd = create_socket()) < 0) {
        return -1;
    }

    if (set_nonblocking(fd) < 0) {
        return -1;
    }

    memset(&sa,0,sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bindaddr && inet_aton(bindaddr, &sa.sin_addr) == 0) {
        qerror("invalid bind address");
        close(fd);
        return -1;
    }
    if (net_listen(fd,(struct sockaddr*)&sa, sizeof(sa)) < 0) {
        close(fd);
        return -1;
    }
    return fd;
}

int qnet_tcp_accept(int listen_fd) {
  int fd = accept(listen_fd, NULL, NULL);
  if (fd == -1 && 
    (errno == EAGAIN || errno == EWOULDBLOCK || 
     errno == EINTR || errno == ECONNABORTED)) {
    return -1;
  }
  qassert(fd != -1);
  set_nonblocking(fd);
  return fd;
}
