/*
 * See Copyright Notice in qnode.h
 */

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "qassert.h"
#include "qlog.h"
#include "qmsg.h"
#include "qmailbox.h"
#include "qmalloc.h"

typedef struct signaler_t {
  int rfd;
  int wfd;
} signaler_t;

static void signal_make_pair(int *rfd, int *wfd) {
  int fds[2];
  int result;
  result = socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
  qcheck(result == 0);
  *wfd = fds[0];
  *rfd = fds[1];
}

static signaler_t* signaler_new() {
  signaler_t *signal = qalloc_type(signaler_t);
  signal_make_pair(&(signal->rfd), &(signal->wfd));
  return signal;
}

static void signaler_destroy(signaler_t *signal) {
  close(signal->rfd);
  close(signal->wfd);
  qfree(signal);
}

static int signaler_get_fd(signaler_t *signal) {
  return signal->rfd;
}

static void signaler_send(signaler_t *signal) {
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

static void signaler_recv(signaler_t *signal) {
  unsigned char dummy;
  ssize_t n;
  n = recv(signal->rfd, &dummy, sizeof(dummy), 0);
  qcheck(n >= 0);
  qcheck(n == sizeof(dummy));
  qcheck(dummy == 0);
}

qmailbox_t* qmailbox_new(qevent_func_t *callback, void *reader) {
  qmailbox_t *box = qalloc_type(qmailbox_t);
  qalloc_assert(box);
  qlist_entry_init(&(box->lists[0]));
  qlist_entry_init(&(box->lists[1]));
  box->write  = &(box->lists[0]);
  box->read   = &(box->lists[1]);
  box->active = 0;
  box->callback = callback;
  box->reader = reader;
  signaler_t *signal = signaler_new();
  qalloc_assert(signal);
  box->signal = signal;
  return box;
}

void qmailbox_destroy(qmailbox_t *box) {
  signaler_destroy(box->signal);
  qfree(box);
}

int qmailbox_active(qengine_t *engine, qmailbox_t *box) {
  int fd = signaler_get_fd(box->signal);
  qevent_func_t *callback = box->callback;
  void *data = box->reader;
  return qengine_add_event(engine, fd, QEVENT_READ, callback, data);
}

static int mailbox_active(qmailbox_t *box, int active) {
  qatomic_t cmp = !active;
  qatomic_t val = active;
  return qatomic_cas(&(box->active), cmp, val);
}

void qmailbox_add(qmailbox_t *box, struct qmsg_t *msg) {
  qinfo("qmailbox_add %p, write: %p", box, box->write);
  /* save the write ptr first cause add_tail below
   * is-not atomic operation and the write ptr maybe changed 
   * */
  qlist_t *p = box->write;
  qlist_add_tail(&(msg->entry), p);
  if (mailbox_active(box, 1) == 0) {
    signaler_send(box->signal);
    qinfo("signaler_send %d", box->active);
  }
  qinfo("empty: %d", qlist_empty(p));
}

int qmailbox_get(qmailbox_t *box, qlist_t **list) {
  qinfo("qmailbox_get %p, active: %d, write: %p", box, box->active, box->write);
  *list = NULL;
  /* first save the read ptr */
  qlist_t *read = box->read;
  /* second change the read ptr to the write ptr */
  qatomic_ptr_xchg(&(box->read), box->write);
  /* last change the write ptr to the read ptr saved before and return to list */
  *list = qatomic_ptr_xchg(&(box->write), read);
  if (mailbox_active(box, 0) == 1) {
    signaler_recv(box->signal);
  }
  qinfo("qmailbox_get %p, active: %d, write: %p", box, box->active, *list);
  qinfo("empty: %d", qlist_empty(*list));
  return qlist_empty(*list);
}
