/*
 * See Copyright Notice in qnode.h
 */

#include <errno.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>
#include "qalloc.h"
#include "qengine.h"
#include "qlog.h"

static int  epoll_init(qengine_t *engine);
static int  epoll_add(qengine_t *engine, int fd, int flags);
static int  epoll_del(qengine_t *engine, int fd, int flags);
static int  epoll_poll(qengine_t *engine, int timeout_ms);
static void epoll_destroy(qengine_t *engine);

const struct qdispatcher_t epoll_dispatcher = {
  "epoll",
  epoll_init,
  epoll_add,
  epoll_del,
  epoll_poll,
  epoll_destroy,
};

typedef struct epoll_t {
  int    fd;
  struct epoll_event events[QMAX_EVENTS];
} epoll_t;

static int
epoll_init(qengine_t *engine) {
  int      i;
  epoll_t *epoll;

  epoll = qcalloc(sizeof(epoll_t));
  if (epoll == NULL) {
    return QERROR;
  }
  for (i = 0; i < QMAX_EVENTS; ++i) {
    memset(&(epoll->events[i]), 0, sizeof(struct epoll_event));
  }

  epoll->fd = epoll_create(1024);
  if (epoll->fd == -1) {
    qerror("epoll_create error: %s", strerror(errno));
    return QERROR;
  }
  engine->data = epoll;

  return QOK;
}

static int
epoll_add(qengine_t *engine, int fd, int flags) {
  int                 events, op;
  epoll_t            *epoll;
  struct epoll_event  event;

  epoll = (epoll_t*)engine->data;
  memset(&event, 0, sizeof(struct epoll_event));

  op = engine->events[fd].flags == QEVENT_NONE ?
        EPOLL_CTL_ADD : EPOLL_CTL_MOD;

  flags |= engine->events[fd].flags;
  events = 0;
  if (flags & QEVENT_READ) {
    events |= EPOLLIN;
  }
  if (flags & QEVENT_WRITE) {
    events |= EPOLLOUT;
  }

  events |= EPOLLET;
  event.events = events;
  event.data.fd = fd;
  if (epoll_ctl(epoll->fd, op, fd, &event) == -1) {
    qerror("epoll_ctl error: %s", strerror(errno));
    return QERROR;
  }

  return QOK;
}

static int
epoll_del(qengine_t *engine, int fd, int delflags) {
  epoll_t            *epoll;
  struct epoll_event  event;
  int                 flags, events;

  epoll  = engine->data;
  flags = engine->events[fd].flags & (~delflags);
  events = 0;
  if (flags & QEVENT_READ) {
    events |= EPOLLIN;
  }
  if (flags & QEVENT_WRITE) {
    events |= EPOLLOUT;
  }
  events |= EPOLLET;
  event.data.fd = fd;
  event.events = events;
  if (flags != QEVENT_NONE) {
    return epoll_ctl(epoll->fd, EPOLL_CTL_MOD, fd, &event);
  } else {
    return epoll_ctl(epoll->fd, EPOLL_CTL_DEL, fd, &event);
  }

  /* nerver reach here */
  return QOK;
}

static int
epoll_poll(qengine_t *engine, int timeout_ms) {
  int                 num;
  int                 flags;
  int                 i;
  epoll_t            *epoll;
  struct epoll_event *event;

  epoll = (epoll_t*)engine->data;
  num = epoll_wait(epoll->fd, &(epoll->events[0]), QMAX_EVENTS, timeout_ms);

  if (num > 0) {
    for (i = 0; i < num; i++) {
      event = epoll->events + i;

      if (event->events & EPOLLIN) {
        flags |= QEVENT_READ;
      }
      if (event->events & EPOLLOUT) {
        flags |= QEVENT_WRITE;
      }
      if (event->events & (EPOLLERR | EPOLLHUP)) {
        flags |= QEVENT_ERROR;
      }
      engine->active_events[i].fd = event->data.fd;
      engine->active_events[i].flags = flags;
    }
  }

  return num;
}

static void
epoll_destroy(qengine_t *engine) {
  epoll_t *epoll;

  epoll = (epoll_t*)engine->data;
  close(epoll->fd);
  qfree(epoll);
}
