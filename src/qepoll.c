/*
 * See Copyright Notice in qnode.h
 */

#include <errno.h>
#include <string.h>
#include <sys/epoll.h>
#include "qengine.h"
#include "qlog.h"
#include "qmalloc.h"

static int epoll_init (qengine_t *engine);
static int epoll_add  (struct qengine_t *engine, int fd, int flags);
static int epoll_del  (struct qengine_t *engine, int fd, int flags);
static int epoll_poll (qengine_t *engine, qtime_t timeout_ms);

const struct qdispatcher_t epoll_dispatcher = {
  "epoll",
  epoll_init,
  epoll_add,
  epoll_del,
  epoll_poll,
};

typedef struct epoll_t {
  int fd;
  struct epoll_event events[QMAX_EVENTS];
} epoll_t;

static int epoll_init(struct qengine_t *engine) {
  epoll_t *epoll = qalloc_type(epoll_t);
  if (epoll == NULL) {
    return -1;
  }

  epoll->fd = epoll_create(1024);
  if (epoll->fd == -1) {
    qerror("epoll_create error: %s", strerror(errno));
    return -1;
  }
  engine->data = epoll;
  return 0;
}

static int epoll_add(struct qengine_t *engine, int fd, int flags) {
  epoll_t *epoll = engine->data;
  struct epoll_event event;
  int events = 0;

  int op = engine->events[fd].flags == QEVENT_NONE ?  EPOLL_CTL_ADD : EPOLL_CTL_MOD;

  flags |= engine->events[fd].flags;
  if (flags & QEVENT_READ) {
    events |= EPOLLIN;
  }
  if (flags & QEVENT_WRITE) {
    events |= EPOLLOUT;
  }

  event.events = events;
  event.data.fd = fd;
  if (epoll_ctl(epoll->fd, op, fd, &event) == -1) {
    qerror("epoll_ctl error: %s", strerror(errno));
    return -1;
  }
  return 0;
}

static int epoll_del(struct qengine_t *engine, int fd, int delflags) {
  epoll_t *epoll  = engine->data;
  struct epoll_event event;
  int flags = engine->events[fd].flags & (~delflags);
  int events = 0;

  if (flags & QEVENT_READ) {
    events |= EPOLLIN;
  }
  if (flags & QEVENT_WRITE) {
    events |= EPOLLOUT;
  }
  event.data.fd = fd;
  event.events = events;
  if (flags != QEVENT_NONE) {
    epoll_ctl(epoll->fd, EPOLL_CTL_MOD, fd, &event);
  } else {
    epoll_ctl(epoll->fd, EPOLL_CTL_DEL, fd, &event);
  }
  return 0;
}

static int epoll_poll (qengine_t *engine, qtime_t timeout_ms) {
  epoll_t *epoll = engine->data;
  int num = 0;

  num = epoll_wait(epoll->fd, epoll->events, QMAX_EVENTS, timeout_ms);

  if (num > 0) {
    int i;
    for (i = 0; i < num; i++) {
      int flags = 0;
      struct epoll_event *event = epoll->events + i;

      if (event->events & EPOLLIN) {
        flags |= QEVENT_READ;
      }
      if (event->events & EPOLLOUT) {
        flags |= QEVENT_WRITE;
      }
      engine->active_events[i].fd = event->data.fd;
      engine->active_events[i].flags = flags;
    }
  }
  return num;
}
