/*
 * See Copyright Notice in qnode.h
 */

#include "qevent.h"
#include "qengine.h"
#include "qlog.h"

void
qevent_init(qevent_t *event, int fd,
            qevent_pt *read, qevent_pt *write,
            void *data) {
  event->fd     = fd;
  event->events = 0;
  event->flags  = 0;
  event->error  = 0;
  event->read   = read;
  event->write  = write;
  event->data   = data;
}

int
qevent_add(qengine_t* engine, qevent_t *event, int flags) {
  if (event->fd > engine->size) {
    if (qengine_expand(engine) == NULL) {
      return QERROR;
    }
  }
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
