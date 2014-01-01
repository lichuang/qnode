/*
 * See Copyright Notice in qnode.h
 */

#ifdef USE_MACOSX

#include <unistd.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include "qalloc.h"
#include "qengine.h"
#include "qlog.h"

static int  kqueue_init(qengine_t *engine);
static int  kqueue_add(qengine_t *engine, int fd, int flags);
static int  kqueue_del(qengine_t *engine, int fd, int flags);
static int  kqueue_poll(qengine_t *engine, int timeout_ms);
static int  kqueue_expand(qengine_t *engine);
static void kqueue_destroy(qengine_t *engine);

const struct qdispatcher_t kqueue_dispatcher = {
  "kqueue",
  kqueue_init,
  kqueue_add,
  kqueue_del,
  kqueue_poll,
  kqueue_expand,
  kqueue_destroy,
};

typedef struct qkqueue_t {
  int kqfd;
  struct kevent *events;
} qkqueue_t;

static int
kqueue_init(qengine_t *engine) {
  qkqueue_t *kq;

  kq = qcalloc(sizeof(qkqueue_t));
  if (kq == NULL) {
    return QERROR;
  }

  kq->events = qalloc(sizeof(struct kevent) * engine->size);
  if (kq->events == NULL) {
    return QERROR;
  }
  kq->kqfd = kqueue();
  if (kq->kqfd == -1) {
    qfree(kq->events);
    qfree(kq);
    return QERROR;
  }
  engine->data = kq;

  return QOK;
}

static int
kqueue_add(qengine_t *engine, int fd, int flags) {
  qkqueue_t     *kqueue;
  struct kevent  event;

  kqueue = engine->data;
  if (flags & QEVENT_READ) {
    EV_SET(&event, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    if (kevent(kqueue->kqfd, &event, 1, NULL, 0, NULL) == -1) {
      return QERROR;
    }
  }
  if (flags & QEVENT_WRITE) {
    EV_SET(&event, fd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
    if (kevent(kqueue->kqfd, &event, 1, NULL, 0, NULL) == -1) {
      return QERROR;
    }
  }

  return QOK;
}

static int
kqueue_del(qengine_t *engine, int fd, int flags) {
  qkqueue_t     *kqueue;
  struct kevent  event;

  kqueue = engine->data;
  if (flags & QEVENT_READ) {
    EV_SET(&event, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    if (kevent(kqueue->kqfd, &event, 1, NULL, 0, NULL) == -1) {
      return QERROR;
    }
  }
  if (flags & QEVENT_WRITE) {
    EV_SET(&event, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
    if (kevent(kqueue->kqfd, &event, 1, NULL, 0, NULL) == -1) {
      return QERROR;
    }
  }

  return QOK;
}

static int
kqueue_poll(qengine_t *engine, int timeout_ms) {
  qkqueue_t      *kqueue;
  struct timespec timeout;
  int             num;

  kqueue = engine->data;
  num    = 0;

  timeout.tv_sec  = timeout_ms / 1000;
  timeout.tv_nsec = (timeout_ms - 1000 * timeout.tv_sec) * 1000;
  num = kevent(kqueue->kqfd, NULL, 0, kqueue->events,
               engine->size, &timeout);

  if (num > 0) {
    int            i;
    for (i = 0; i < num; ++i) {
      int             flags = 0;
      int             fd    = -1;
      struct kevent  *event = NULL;
      qevent_t       *ev    = NULL;

      event = kqueue->events + i;
      fd    = event->ident;

      ev = engine->events[fd];
      if (event->filter & EVFILT_READ) {
        flags |= QEVENT_READ;
      }
      if (event->filter & EVFILT_WRITE) {
        flags |= QEVENT_WRITE;
      }
      /*
      if (event->events & (EPOLLERR | EPOLLHUP)) {
        flags = QEVENT_ERROR;
        ev->error = 1;
      }
      */
      ev->flags = flags;
      qlist_add_tail(&ev->active_entry, &engine->active);
    }
  }

  return num;   
}

static int
kqueue_expand(qengine_t *engine) {
  qkqueue_t *kqueue;
  void      *data;

  kqueue = engine->data;
  data  = qrealloc(kqueue->events, sizeof(struct kevent) * engine->size);
  if (data == NULL) {
    return QERROR;
  }
  kqueue->events = data;

  return QOK;
}

static void
kqueue_destroy(qengine_t *engine) {
  qkqueue_t      *kqueue;

  kqueue = engine->data;
  close(kqueue->kqfd);
  qfree(kqueue->events);
  qfree(kqueue);
}

#endif
