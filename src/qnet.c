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
#include "qbuffer.h"
#include "qdefines.h"
#include "qdescriptor.h"
#include "qlog.h"
#include "qnet.h"

static int create_listen_socket();
static int set_nonblocking(int fd);

static int
create_listen_socket() {
  int fd, on;

  on = 1;
  if ((fd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
    qerror("create socket error: %s", strerror(errno));
    return -1;
  }

  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
    qerror("setsockopt SO_REUSEADDR error: %s", strerror(errno));
    return -1;
  }

  return fd;
}

static int
set_nonblocking(int fd) {
  int flags;

  if ((flags = fcntl(fd, F_GETFL)) == -1) {
    qerror("fcntl(F_GETFL) error: %s", strerror(errno));
    return -1;
  }
  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
    qerror("fcntl(F_SETFL,O_NONBLOCK) error: %s", strerror(errno));
    return -1;
  }
  return 0;
}

int
qnet_tcp_listen(int port, const char *bindaddr) {
  int                 fd;
  struct sockaddr_in  sa;

  UNUSED(bindaddr);

  if ((fd = create_listen_socket()) < 0) {
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

  if (bind(fd, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
    qerror("bind error: %s", strerror(errno));
    close(fd);
    return -1;
  }

  if (listen(fd, 511) == -1) {
    qerror("listen error: %s", strerror(errno));
    close(fd);
    return -1;
  }

  return fd;
}

int
qnet_tcp_accept(int listen_fd, struct sockaddr *addr,
                socklen_t *addrlen) {
  int fd;

  while (1) {
    fd = accept(listen_fd, addr, addrlen);
    if (fd == -1) { 
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return 0;
      }
      if (errno == EINTR) {
        continue;
      }
      qerror("accept error: %s", strerror(errno));
      return -1;
    }
    set_nonblocking(fd);
    break;
  }
  return fd;
}

int
qnet_tcp_recv(struct qdescriptor_t *desc) {
  int                 fd, nbytes, size, n;
  int                 save;
  qbuffer_t          *buffer;
  qtcp_descriptor_t  *tcp;

  tcp = &(desc->data.tcp);
  fd = desc->fd;
  buffer = &(tcp->inbuf);

  nbytes = 0;
  do {
    if (qbuffer_writeable_len(buffer) == 0) {
      qbuffer_extend(buffer, buffer->size * 2);
    }
    if (buffer->data == NULL) {
      return -1;
    }
    size = qbuffer_writeable_len(buffer);
    n = recv(fd, buffer->data + buffer->end, size, 0);
    save = errno;
    qdebug("%d recv nbytes: %d\n", fd, n);

    if (n == -1) { 
      if(save == EAGAIN || save == EWOULDBLOCK) {
        return 0;
      } else if (save == ECONNRESET || save == ECONNREFUSED
        || save == ETIMEDOUT || save == EHOSTUNREACH || save == ENOTCONN) {
        qerror("recv error: %s\n", strerror(save));
        return -1;
      }
    }

    if (n == 0) {
      return -1;
    }
    buffer->end += n;
    buffer->data[buffer->end] = '\0';
    nbytes += n;
  } while (save != EINTR);

  return nbytes;
}

int
qnet_tcp_send(qdescriptor_t *desc) {
  int                 fd, nbytes;
  qbuffer_t          *buffer;
  qtcp_descriptor_t  *tcp;

  tcp = &(desc->data.tcp);
  fd = desc->fd;
  buffer = &(tcp->outbuf);

  nbytes = send(fd, buffer->data + buffer->start,
                buffer->end - buffer->start, 0);

  if (nbytes == -1 &&
      (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
    return 0;
  }

  if (nbytes == -1 && (errno == ECONNRESET || errno == EPIPE)) {
    return -1;
  }

  buffer->start += nbytes;
  return nbytes;
}
