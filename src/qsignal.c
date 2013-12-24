/*
 * See Copyright Notice in qnode.h
 */

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "qalloc.h"
#include "qassert.h"
#include "qcore.h"
#include "qdefines.h"
#include "qengine.h"
#include "qmailbox.h"
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

int
qsignal_init(qsignal_t *signal, qmailbox_t *box, qengine_t *engine) {
  int fds[2], result;

  signal->box = box;
  signal->active = 0;

  result = socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
  if (result != 0) {
    return QERROR;
  }

  signal->wfd = fds[0];
  signal->rfd = fds[1];

  fcntl(fds[0], F_SETFL, fcntl(fds[0], F_GETFL) | O_NONBLOCK);
  fcntl(fds[1], F_SETFL, fcntl(fds[1], F_GETFL) | O_NONBLOCK);

  qevent_init(&signal->event, signal->rfd,
              signal_handle, NULL, signal);
  qevent_add(engine, &signal->event, QEVENT_READ);
  return QOK;
}

void
qsignal_free(qsignal_t *signal) {
  close(signal->wfd);
  close(signal->rfd);
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
    n = write(signal->wfd, &dummy, sizeof(dummy));
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
    n = read(signal->rfd, &dummy, sizeof(dummy));
    if (n == -1 && errno == EINTR) {
      continue;
    }
    break;
  }
}
