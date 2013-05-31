/*
 * See Copyright Notice in qnode.h
 */

#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "qalloc.h"
#include "qassert.h"
#include "qengine.h"
#include "qsignal.h"

static void
signal_handle(int fd, int flags, void *data) {
  qsignal_t  *signal;

  UNUSED(fd);
  UNUSED(flags);

  signal = (qsignal_t*)data;
  qsignal_recv(signal);
  qsignal_active(signal, 0);
  qmailbox_handle(signal->box);
}

void
qsignal_init(qsignal_t *signal, qmailbox_t *box) {
  int fds[2], result;

  signal->box = box;
  signal->active = 0;

  result = socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

  qassert(result == 0);
  signal->wfd = fds[0];
  signal->rfd = fds[1];
  qengine_add_event(box->engine, signal->rfd,
                    QEVENT_READ, signal_handle, signal);
}

int
qsignal_active(qsignal_t *signal, int active) {
  qatomic_t cmp = !active;
  qatomic_t val = active;
  return qatomic_cas(&(signal->active), cmp, val);
}

void
qsignal_send(qsignal_t *signal) {
  char    dummy;
  ssize_t n;

  dummy = 0;
  while (1) {
    n = send(signal->wfd, &dummy, sizeof(dummy), 0);
    if (n == -1 && errno == EINTR) {
      continue;
    }
    break;
  }
}

void
qsignal_recv(qsignal_t *signal) {
  char    dummy;
  ssize_t n;

  while (1) {
    n = recv(signal->rfd, &dummy, sizeof(dummy), 0);
    if (n == -1 && errno == EINTR) {
      continue;
    }
    break;
  }
}
