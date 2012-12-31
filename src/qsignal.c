/*
 * See Copyright Notice in qnode.h
 */

#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "qassert.h"
#include "qmalloc.h"
#include "qsignal.h"

static void signal_make_pair(int *rfd, int *wfd) {
  int fds[2];
  int result;
  result = socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
  qcheck(result == 0);
  *wfd = fds[0];
  *rfd = fds[1];
}

qsignal_t* qsignal_new() {
  qsignal_t *signal = qalloc_type(qsignal_t);
  signal_make_pair(&(signal->rfd), &(signal->wfd));
  signal->active = 0;
  return signal;
}

void qsignal_destroy(qsignal_t *signal) {
  close(signal->rfd);
  close(signal->wfd);
  qfree(signal);
}

int qsignal_get_fd(qsignal_t *signal) {
  return signal->rfd;
}

int qsignal_active(qsignal_t *signal, int active) {
  qatomic_t cmp = !active;
  qatomic_t val = active;
  return qatomic_cas(&(signal->active), cmp, val);
}

void qsignal_send(qsignal_t *signal) {
  unsigned char dummy = 0;
  while (1) {
    ssize_t n = send(signal->wfd, &dummy, sizeof(dummy), 0);
    if (n == -1 && errno == EINTR) {
      continue;
    }
    qcheck(n == sizeof(dummy));
    break;
  }
}

void qsignal_recv(qsignal_t *signal) {
  unsigned char dummy;
  ssize_t n;
  n = recv(signal->rfd, &dummy, sizeof(dummy), 0);
  qcheck(n >= 0);
  qcheck(n == sizeof(dummy));
  qcheck(dummy == 0);
}
