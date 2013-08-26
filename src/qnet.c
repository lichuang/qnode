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
#include "qlog.h"
#include "qnet.h"
#include "qsocket.h"

static int create_listen_socket();
static int set_nonblocking(int fd);

static int
create_listen_socket() {
  int fd, on;

  on = 1;
  if ((fd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
    qerror("create socket error: %s", strerror(errno));
    return QERROR;
  }

  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
    qerror("setsockopt SO_REUSEADDR error: %s", strerror(errno));
    return QERROR;
  }

  return fd;
}

static int
set_nonblocking(int fd) {
  int flags;

  if ((flags = fcntl(fd, F_GETFL)) == -1) {
    qerror("fcntl(F_GETFL) error: %s", strerror(errno));
    return QERROR;
  }
  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
    qerror("fcntl(F_SETFL,O_NONBLOCK) error: %s", strerror(errno));
    return QERROR;
  }

  return QOK;
}

int
qnet_tcp_listen(int port, const char *addr, int *error) {
  int                 fd;
  struct sockaddr_in  sa;

  if ((fd = create_listen_socket()) < 0) {
    return QERROR;
  }

  if (set_nonblocking(fd) < 0) {
    return QERROR;
  }

  memset(&sa,0,sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  *error = 0;
  if (inet_aton(addr, &sa.sin_addr) == 0) {
    qerror("invalid bind address");
    *error = errno;
    close(fd);
    return QERROR;
  }

  if (bind(fd, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
    *error =  errno;
    qerror("bind error: %s", strerror(*error));
    close(fd);
    return QERROR;
  }

  if (listen(fd, 511) == -1) {
    *error =  errno;
    qerror("listen error: %s", strerror(*error));
    close(fd);
    return QERROR;
  }

  return fd;
}

int
qnet_tcp_accept(int listen_fd, struct sockaddr *addr,
                socklen_t *addrlen, int *error) {
  int fd;

  while (1) {
    fd = accept(listen_fd, addr, addrlen);
    if (fd == -1) { 
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return QOK;
      }
      if (errno == EINTR) {
        continue;
      }
      *error = errno;
      qinfo("accept error: %s", strerror(*error));
      return QERROR;
    }
    set_nonblocking(fd);
    break;
  }
  return fd;
}

int
qnet_tcp_recv(qsocket_t *socket, int *error) {
  int                 fd, save;
  int                 nbytes, size, n;
  qbuffer_t          *buffer;

  fd = socket->fd;
  buffer = socket->in;

  nbytes = 0;
  /*
   * recv data from tcp socket until:
   *  1) some error occur or
   *  2) received data less then required size
   *    (means there is no data in the tcp stack buffer)
   */
  do {
    if (qbuffer_wlen(buffer) == 0) {
      qbuffer_extend(buffer, buffer->size * 2);
    }
    if (buffer->data == NULL) {
      return QERROR;
    }

    size = qbuffer_wlen(buffer);
    do {
      n = recv(fd, qbuffer_writeable(buffer), size, 0);
      save = errno;
    } while (save == EINTR);

    if (n == -1) { 
      if(save == EAGAIN || save == EWOULDBLOCK) {
        /* non-blocking mode, there is no data in the buffer now */
        break;
      } else if (save != EINTR) {
        /* some error has occured */
        qinfo("recv from %s error: %s", socket->peer, strerror(save));
        *error = save;
        return QERROR;
      }
    }

    /* socket has been closed */
    if (n == 0) {
      qinfo("socket from %s closed", socket->peer);
      *error = save;
      return QERROR;
    }

    buffer->end += n;
    buffer->data[buffer->end] = '\0';
    nbytes += n;
    qinfo("%s recv:%d, total:%d", socket->peer, n, nbytes);
    /* while more tcp stack buffer data available continue */
  } while (n == size);

  return nbytes;
}

int
qnet_tcp_send(qsocket_t *socket, int *error) {
  int                 fd, nbytes;
  int                 save, n, size;
  qbuffer_t          *buffer;

  fd = socket->fd;
  buffer = socket->out;

  size = qbuffer_rlen(buffer);
  nbytes = 0;
  while(size > 0) {
    do {
      n = send(fd, qbuffer_readable(buffer), size, MSG_NOSIGNAL);
      save = errno;
    } while (save == EINTR);

    if (n == -1) {
      if (save == EAGAIN || save == EWOULDBLOCK) {
        return nbytes;
      } else {
        qinfo("send to %s error: %s", socket->peer, strerror(save));
        *error = save;
        return QERROR;
      }
    }

    if (n == 0) {
      qinfo("socket from %s closed", socket->peer);
      *error = save;
      return QERROR;
    }

    buffer->start += n;
    nbytes += n;
    size -= n;
    qinfo("%s sent:%d, total:%d", socket->peer, n, nbytes);
  }

  return nbytes;
}

void
qnet_close(int fd) {
  close(fd);
}
