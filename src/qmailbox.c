/*
 * See Copyright Notice in qnode.h
 */

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "qassert.h"
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
  qnode_check(result == 0);
  *wfd = fds[0];
  *rfd = fds[1];
}

static signaler_t* signaler_new() {
  signaler_t *signal = qnode_alloc_type(signaler_t);
  signal_make_pair(&(signal->rfd), &(signal->wfd));
  return signal;
}

static void signaler_destroy(signaler_t *signal) {
  close(signal->rfd);
  close(signal->wfd);
  qnode_free(signal);
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
    qnode_check(n == sizeof(dummy));
    break;
  }
}

static void signaler_recv(signaler_t *signal) {
  unsigned char dummy;
  ssize_t n;
  n = recv(signal->rfd, &dummy, sizeof(dummy), 0);
  qnode_check(n >= 0);
  qnode_check(n == sizeof(dummy));
  qnode_check(dummy == 0);
}

qnode_mailbox_t* qnode_mailbox_new(qnode_event_func_t *callback, void *reader) {
  qnode_mailbox_t *box = qnode_alloc_type(qnode_mailbox_t);
  qnode_alloc_assert(box);
  qnode_list_entry_init(&(box->lists[0]));
  qnode_list_entry_init(&(box->lists[1]));
  box->write  = &(box->lists[0]);
  box->read   = &(box->lists[1]);
  box->active = 0;
  box->callback = callback;
  box->reader = reader;
  signaler_t *signal = signaler_new();
  qnode_alloc_assert(signal);
  box->signal = signal;
  return box;
}

void qnode_mailbox_destroy(qnode_mailbox_t *box) {
  signaler_destroy(box->signal);
  qnode_free(box);
}

int qnode_mailbox_active(qnode_engine_t *engine, qnode_mailbox_t *box) {
  int fd = signaler_get_fd(box->signal);
  qnode_event_func_t *callback = box->callback;
  void *data = box->reader;
  return qnode_engine_add_event(engine, fd, QNODE_EVENT_READ, callback, data);
}

static int mailbox_active(qnode_mailbox_t *box, int active) {
  qnode_atomic_t cmp = !active;
  qnode_atomic_t val = active;
  return qnode_atomic_cas(&(box->active), &cmp, &val);
}

void qnode_mailbox_add(qnode_mailbox_t *box, struct qnode_msg_t *msg) {
  /* save the write ptr first cause add_tail below
   * is-not atomic operation and the write ptr maybe changed 
   * */
  qnode_list_t *p = box->write;
  qnode_list_add_tail(&(msg->entry), p);
  if (mailbox_active(box, 1) == 0) {
    signaler_send(box->signal);
  }
}

int qnode_mailbox_get(qnode_mailbox_t *box, struct qnode_list_t *list) {
  /* first save the read ptr */
  qnode_list_t *read = box->read;
  /* second change the read ptr to the write ptr */
  qnode_atomic_ptr_xchg(box->read, box->write);
  /* last change the write ptr to the read ptr saved before and return to list */
  list = qnode_atomic_ptr_xchg(box->write, read);
  if (mailbox_active(box, 0) == 1) {
    signaler_recv(box->signal);
  }
  return (qnode_list_empty(list));
}
