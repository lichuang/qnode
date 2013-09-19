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

qengine_t*
qengine_new() {
  int         i;
  qengine_t  *engine;

  engine = qcalloc(sizeof(qengine_t));
  if (engine == NULL) {
    goto error;
  }
  engine->max_fd = 0;
  engine->dispatcher = &(epoll_dispatcher);
  if (engine->dispatcher->init(engine) != QOK) {
    goto error;
  }
  engine->events = qalloc(sizeof(qevent_t*) * QMAX_EVENTS);
  if (engine->events == NULL) {
    goto error;
  }
  for (i = 0; i < QMAX_EVENTS; ++i) {
    engine->events[i] = NULL;
  }
  qtimer_manager_init(&engine->timer_mng, engine);
  engine->quit = 0;
  qlist_entry_init(&(engine->active));

  return engine;

error:
  return NULL;
}

int
qevent_add(qengine_t* engine, qevent_t *event, int flags) {
  /*
  if (fd > QMAX_EVENTS) {
    qerror("extends max fd");
    return QERROR;
  }
  */
  if (engine->dispatcher->add(engine, event->fd, flags) < 0) {
    qerror("add event error");
    return QERROR;
  }
  event->events |= flags;
  engine->events[event->fd] = event;
  if (event->fd > engine->max_fd) {
    engine->max_fd = event->fd;
  }

  return QOK;
}

int
qevent_del(qengine_t* engine, qevent_t *event, int flags) {
  int       i;

  /*
  if (fd > QMAX_EVENTS) {
    qerror("extends max fd");
    return QERROR;
  }
  */
  if (engine->events[event->fd] == NULL) {
    return QOK;
  }
  if (flags == QEVENT_NONE) {
    return QERROR;
  }
  if (event->fd == engine->max_fd && event->flags == QEVENT_NONE) {
    for (i = engine->max_fd - 1; i > 0; --i) {
      if (engine->events[i]->events != 0) {
        engine->max_fd = i;
        break;
      }
    }
  }
  if (engine->dispatcher->del(engine, event->fd, flags) < 0) {
    return QERROR;
  }
  event->events = event->events & (~flags);
  if (event->events == 0) {
    engine->events[event->fd] = NULL;
  }

  return QOK;
}

int
qengine_loop(qengine_t* engine) {
  int       nexttime;
  int       flags;
  int       fd;
  qevent_t *event;
  qlist_t  *pos, *next;

  while (1) {
    if (engine->quit) {
      break;
    }
    nexttime = qtimer_next(&engine->timer_mng);
    engine->dispatcher->poll(engine, nexttime);

    for (pos = engine->active.next; pos != &engine->active; pos = next) {
      event = qlist_entry(pos, qevent_t, active_entry);
      next  = pos->next;
      qlist_del_init(&(event->active_entry));
      flags = event->flags;
      event->flags = 0;
      fd = event->fd;

      if (flags & QEVENT_READ) {
        event->read(fd, flags, event->data);
      }
      if (flags & QEVENT_WRITE) {
        event->write(fd, flags, event->data);
      }
      if (event->error || (flags & QEVENT_ERROR)) {
        qevent_del(engine, event, event->events);
        close(fd);
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
