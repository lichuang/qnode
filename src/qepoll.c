/*
 * See Copyright Notice in qnode.h
 */

#ifdef USE_LINUX

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
static int  epoll_expand(qengine_t *engine);
static void epoll_destroy(qengine_t *engine);

const struct qdispatcher_t epoll_dispatcher = {
  "epoll",
  epoll_init,
  epoll_add,
  epoll_del,
  epoll_poll,
  epoll_expand,
  epoll_destroy,
};

typedef struct epoll_t {
  int    fd;
  struct epoll_event *events;
} epoll_t;

static int
epoll_init(qengine_t *engine) {
  epoll_t *epoll;

  epoll = qcalloc(sizeof(epoll_t));
  if (epoll == NULL) {
    return QERROR;
  }
  epoll->events = qalloc(sizeof(struct epoll_event) * engine->size);
  if (epoll->events == NULL) {
    return QERROR;
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
epoll_expand(qengine_t *engine) {
  epoll_t *epoll;
  void    *data;

  epoll = (epoll_t*)engine->data;
  data  = qrealloc(epoll->events, sizeof(struct epoll_event) * engine->size);
  if (data == NULL) {
    return QERROR;
  }
  epoll->events = data;

  return QOK;
}

static int
epoll_add(qengine_t *engine, int fd, int flags) {
  int                 events, op;
  epoll_t            *epoll;
  struct epoll_event  event;

  epoll = (epoll_t*)engine->data;
  memset(&event, 0, sizeof(struct epoll_event));

  if (engine->events[fd] == NULL) {
    op = EPOLL_CTL_ADD;
  } else {
    op = EPOLL_CTL_MOD;
  }

  events = 0;
  if (flags & QEVENT_READ) {
    events |= EPOLLIN;
  }
  if (flags & QEVENT_WRITE) {
    events |= EPOLLOUT;
  }

  //events |= EPOLLET;
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
  flags = engine->events[fd]->events & (~delflags);
  events = 0;
  if (flags & QEVENT_READ) {
    events |= EPOLLIN;
  }
  if (flags & QEVENT_WRITE) {
    events |= EPOLLOUT;
  }
  //events |= EPOLLET;
  event.data.fd = fd;
  event.events = events;
  if (flags != 0) {
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
  int                 i, fd;
  epoll_t            *epoll;
  struct epoll_event *event;
  qevent_t           *ev;

  epoll = (epoll_t*)engine->data;
  num = epoll_wait(epoll->fd, &(epoll->events[0]), engine->size, timeout_ms);

  if (num > 0) {
    for (i = 0; i < num; i++) {
      event = epoll->events + i;
      fd    = event->data.fd;

      ev = engine->events[fd];
      if (event->events & EPOLLIN) {
        flags |= QEVENT_READ;
      }
      if (event->events & EPOLLOUT) {
        flags |= QEVENT_WRITE;
      }
      if (event->events & (EPOLLERR | EPOLLHUP)) {
        flags = QEVENT_ERROR;
        ev->error = 1;
      }
      ev->flags = flags;
      qlist_add_tail(&ev->active_entry, &engine->active);
    }
  }

  return num;
}

static void
epoll_destroy(qengine_t *engine) {
  epoll_t *epoll;

  epoll = (epoll_t*)engine->data;
  close(epoll->fd);
  qfree(epoll->events);
  qfree(epoll);
}
#endif
