/*
 * See Copyright Notice in qnode.h
 */

#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "qalloc.h"
#include "qassert.h"
#include "qchannel.h"

static void channel_make_pair(int *rfd, int *wfd) {
  int fds[2], result;

  result = socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

  qassert(result == 0);
  *wfd = fds[0];
  *rfd = fds[1];
}

qchannel_t* qchannel_new() {
  qchannel_t *channel;

  channel = qalloc(sizeof(qchannel_t));
  if (channel == NULL) {
    return NULL;
  }
  channel_make_pair(&(channel->rfd), &(channel->wfd));
  channel->active = 0;

  return channel;
}

int qchannel_get_fd(qchannel_t *channel) {
  return channel->rfd;
}

int qchannel_active(qchannel_t *channel, int active) {
  qatomic_t cmp = !active;
  qatomic_t val = active;
  return qatomic_cas(&(channel->active), cmp, val);
}

void qchannel_send(qchannel_t *channel) {
  unsigned char dummy;

  dummy = 0;
  while (1) {
    ssize_t n = send(channel->wfd, &dummy, sizeof(dummy), 0);
    if (n == -1 && errno == EINTR) {
      continue;
    }
    //qassert(n == sizeof(dummy));
    break;
  }
}

void qchannel_recv(qchannel_t *channel) {
  unsigned char dummy;
  ssize_t n;

  n = recv(channel->rfd, &dummy, sizeof(dummy), 0);
  /*
  qassert(n >= 0);
  qassert(n == sizeof(dummy));
  qassert(dummy == 0);
  */
}
