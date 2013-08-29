/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "qalloc.h"
#include "qassert.h"
#include "qdefines.h"
#include "qengine.h"
#include "qlog.h"
#include "qthread_log.h"

static const int QRETIRED_FD = -1;

extern const qdispatcher_t epoll_dispatcher;

static void init_event(qevent_t *event);

static void
init_event(qevent_t *event) {
  event->fd = QRETIRED_FD;
  event->flags = 0;
  event->read = event->write = NULL;
  event->data = NULL;
}

qengine_t*
qengine_new() {
  int         i;
  qengine_t  *engine;
  qevent_t   *event;

  engine = qcalloc(sizeof(qengine_t));
  if (engine == NULL) {
    goto error;
  }
  engine->max_fd = 0;
  engine->dispatcher = &(epoll_dispatcher);
  if (engine->dispatcher->init(engine) != QOK) {
    goto error;
  }
  engine->events = qalloc(sizeof(qevent_t) * QMAX_EVENTS);
  if (engine->events == NULL) {
    goto error;
  }
  engine->active_events = qalloc(sizeof(qevent_t) * QMAX_EVENTS);
  if (engine->active_events == NULL) {
    goto error;
  }
  for (i = 0; i < QMAX_EVENTS; ++i) {
    event = &(engine->events[i]);
    init_event(event);
    event = &(engine->active_events[i]);
    init_event(event);
  }
  qtimer_manager_init(&engine->timer_mng, engine);
  engine->quit = 0;

  return engine;

error:
  return NULL;
}

int
qengine_add_event(qengine_t *engine, int fd, int flags,
                  qevent_pt *callback, void *data) {
  qevent_t *event;

  if (fd > QMAX_EVENTS) {
    qerror("extends max fd");
    return QERROR;
  }
  event = &(engine->events[fd]);
  if (engine->dispatcher->add(engine, fd, flags) < 0) {
    qerror("add event error");
    return QERROR;
  }
  event->fd = fd;
  event->flags |= flags;
  event->data = data;
  if (flags & QEVENT_READ) {
    event->read = callback;
  }
  if (flags & QEVENT_WRITE) {
    event->write = callback;
  }
  if (fd > engine->max_fd) {
    engine->max_fd = fd;
  }

  return QOK;
}

int
qengine_del_event(qengine_t* engine, int fd, int flags) {
  int       i;
  qevent_t *event;

  if (fd > QMAX_EVENTS) {
    qerror("extends max fd");
    return QERROR;
  }
  if (flags == QEVENT_NONE) {
    return QERROR;
  }
  event = &(engine->events[fd]);
  event->flags = event->flags & (~flags);
  if (fd == engine->max_fd && event->flags == QEVENT_NONE) {
    for (i = engine->max_fd - 1; i > 0; --i) {
      if (engine->events[i].flags != QEVENT_NONE) {
        engine->max_fd = i;
        break;
      }
    }
  }
  if (engine->dispatcher->del(engine, fd, flags) < 0) {
    return QERROR;
  }

  return QOK;
}

int
qengine_loop(qengine_t* engine) {
  int       num, i;
  int       next;
  int       flags;
  int       fd;
  int       read;
  qevent_t *event;

  while (1) {
    if (engine->quit) {
      break;
    }
    next = qtimer_next(&engine->timer_mng);
    num  = engine->dispatcher->poll(engine, next);
    for (i = 0; i < num; i++) {
      event = &(engine->events[engine->active_events[i].fd]);
      flags = engine->active_events[i].flags;
      fd = engine->active_events[i].fd;
      read = 0;

      if (event->flags & flags & QEVENT_READ) {
        read = 1;
        if (event->read) {
          event->read(fd, flags, event->data);
        }
      }
      if (event->flags & flags & QEVENT_WRITE) {
        if ((!read || event->write != event->read) &&
            event->write != NULL) {
          event->write(fd, flags, event->data);
        }
      }
      if (event->flags & flags & QEVENT_ERROR) {
        qengine_del_event(engine, fd, event->flags);
        close(fd);
        init_event(event);
      }
    }
    qtimer_process(&(engine->timer_mng));
  }

  return QOK;
}

void
qengine_destroy(qengine_t *engine) {
  qtimer_manager_free(&(engine->timer_mng));
  engine->dispatcher->destroy(engine);
  qfree(engine->events);
  qfree(engine->active_events);
  qfree(engine);
}

qid_t
qengine_add_timer(qengine_t* engine, uint32_t timeout_ms,
                  qtimer_pt *func, qtimer_destroy_pt *destroy,
                  int cycle, void *data) {
  return qtimer_add(&engine->timer_mng, timeout_ms, func,
                    destroy, cycle, data);
}

int
qengine_del_timer(qengine_t* engine, qid_t id) {
  return qtimer_del(&engine->timer_mng, id);
}
