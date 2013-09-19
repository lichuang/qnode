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
  engine->events = qalloc(sizeof(qevent_t*) * QINIT_EVENTS);
  if (engine->events == NULL) {
    goto error;
  }
  engine->size = QINIT_EVENTS;
  engine->dispatcher = &(epoll_dispatcher);
  if (engine->dispatcher->init(engine) != QOK) {
    goto error;
  }
  for (i = 0; i < QINIT_EVENTS; ++i) {
    engine->events[i] = NULL;
  }
  qtimer_manager_init(&engine->timer_mng, engine);
  engine->quit = 0;
  qlist_entry_init(&(engine->active));

  return engine;

error:
  return NULL;
}

qengine_t*
qengine_expand(qengine_t *engine) {
  void *data;

  engine->size += QINIT_EVENTS;
  data = qrealloc(engine->events, sizeof(qevent_t*) * engine->size);
  if (data == NULL) {
    return NULL;
  }
  engine->events = data;
  if (engine->dispatcher->expand(engine) != QOK) {
    return NULL;
  }
  return engine;
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
