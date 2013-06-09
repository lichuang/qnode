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
    qerror("create socket error, %s", strerror(errno));
    return -1;
  }

  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
    qerror("setsockopt SO_REUSEADDR error %s", strerror(errno));
    return -1;
  }

  return fd;
}

static int
set_nonblocking(int fd) {
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
    qerror("listen: %s", strerror(errno));
    close(fd);
    return -1;
  }

  return fd;
}

int
qnet_tcp_accept(int listen_fd,
                struct sockaddr *addr, socklen_t *addrlen) {
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
      return -1;
    }
    set_nonblocking(fd);
    break;
  }
  return fd;
}

int
qnet_tcp_recv(struct qdescriptor_t *desc, uint32_t size) {
  int                 fd, nbytes;
  qbuffer_t          *buffer;
  qtcp_descriptor_t  *tcp;

  tcp = &(desc->data.tcp);
  fd = desc->fd;
  buffer = &(tcp->buffer);

  qbuffer_reserve(buffer, size);
  if (buffer->data == NULL) {
    return -1;
  }
  buffer->len = size;
  nbytes = recv(fd, buffer->data + buffer->pos,
                buffer->size - buffer->pos, 0);

  /*
   * Several errors are OK. When speculative read is being done we may not
   * be able to read a single byte to the socket. Also, SIGSTOP issued
   * by a debugging tool can result in EINTR error.
   */
  if (nbytes == -1
    && (errno == EAGAIN
      || errno == EWOULDBLOCK
      || errno == EINTR)) {
    return 0;
  }

  /*
   * Signal peer failure.
   */
  if (nbytes == -1
    && (errno == ECONNRESET
      || errno == ECONNREFUSED
      || errno == ETIMEDOUT
      || errno == EHOSTUNREACH
      || errno == ENOTCONN)) {
    return -1;
  }

  if (nbytes == 0) {
    return -1;
  }
  buffer->pos += nbytes;
  if (buffer->len == 0) {
    buffer->len =  buffer->pos;
  }
  buffer->data[buffer->pos] = '\0';
  //qinfo("recv: %s", buffer->data);
  return nbytes;
}

int
qnet_tcp_send(struct qdescriptor_t *desc) {
  int                 fd, nbytes;
  qbuffer_t          *buffer;
  qtcp_descriptor_t  *tcp;

  tcp = &(desc->data.tcp);
  fd = desc->fd;
  buffer = &(tcp->buffer);

  nbytes = send(fd, buffer->data + buffer->pos,
                buffer->len - buffer->pos, 0);

  if (nbytes == -1 &&
      (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
    return 0;
  }

  if (nbytes == -1 && (errno == ECONNRESET || errno == EPIPE)) {
    return -1;
  }
  return nbytes;
}
