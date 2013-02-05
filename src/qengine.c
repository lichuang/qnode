/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "qassert.h"
#include "qengine.h"
#include "qlog.h"
#include "qmalloc.h"
#include "qthread_log.h"

#define QRETIRED_FD -1

extern const struct qdispatcher_t epoll_dispatcher;

static void init_qevent(qevent_t *event) {
  event->fd = QRETIRED_FD;
  event->flags = 0;
  event->read = event->write = NULL;
  event->data = NULL;
}

static void init_engine_time(qengine_t *engine) {
  struct tm tm;
  time_t t = time(NULL);
  localtime_r(&t, &tm);
  strftime(engine->time_buff, sizeof(engine->time_buff), "[%m-%d %T]", &tm);
  /*
   * convert to ms
   * */
  engine->now = 1000 * t;
}

qengine_t* qengine_new() {
  qengine_t *engine = qalloc_type(qengine_t);
  engine->max_fd = 0;
  engine->dispatcher = &(epoll_dispatcher);
  if (engine->dispatcher->init(engine) < 0) {
    qfree(engine);
    return NULL;
  }
  engine->events = qalloc_array(qevent_t, QMAX_EVENTS);
  engine->active_events = qalloc_array(qevent_t, QMAX_EVENTS);
  int i = 0;
  for (i = 0; i < QMAX_EVENTS; ++i) {
    qevent_t *event = &(engine->events[i]);
    init_qevent(event);
    event = &(engine->active_events[i]);
    init_qevent(event);
  }
  qtimer_manager_init(&engine->timer_mng, engine);
  init_engine_time(engine);
  return engine;
}

int qengine_add_event(qengine_t *engine, int fd, int flags, qevent_func_t *callback, void *data) {
  if (fd > QMAX_EVENTS) {
    qerror("extends max fd");
    return -1;
  }
  qevent_t *event = &(engine->events[fd]);
  if (engine->dispatcher->add(engine, fd, flags) < 0) {
    qerror("add event error");
    return -1;
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
  return 0;
}

int qengine_del_event(qengine_t* engine, int fd, int flags) {
  if (fd > QMAX_EVENTS) {
    qerror("extends max fd");
    return -1;
  }
  if (flags == QEVENT_NONE) {
    return -1;
  }
  qevent_t *event = &(engine->events[fd]);
  event->flags = event->flags & (~flags);
  if (fd == engine->max_fd && event->flags == QEVENT_NONE) {
    int i;
    for (i = engine->max_fd - 1; i > 0; --i) {
      if (engine->events[i].flags != QEVENT_NONE) {
        engine->max_fd = i;
        break;
      }
    }
  }
  engine->dispatcher->del(engine, fd, flags);
  return 0;
}

/*
static uint32_t time_minus(struct timeval *time1, struct timeval *time2) {
  uint32_t sec  = time1->tv_sec  - time2->tv_sec;
  uint32_t usec = time1->tv_usec - time2->tv_usec;
  return (sec / 1000 + usec * 1000);
}
*/

int qengine_loop(qengine_t* engine) {
  int done = 0;
  int num, i;
  //struct timeval now_time;
  //uint32_t timeout_ms;
  while (!done) {
    init_engine_time(engine);

    //gettimeofday(&now_time, NULL);
    int next = qtimer_next(&engine->timer_mng);
    num = engine->dispatcher->poll(engine, next);
    for (i = 0; i < num; i++) {
      qevent_t *event = &(engine->events[engine->active_events[i].fd]);
      int flags = engine->active_events[i].flags;
      int fd = engine->active_events[i].fd;
      int read = 0;

      if (event->flags & flags & QEVENT_READ) {
        read = 1;
        event->read(fd, flags, event->data);
      }
      if (event->flags & flags & QEVENT_WRITE) {
        if (!read || event->write != event->read) {
          event->write(fd, flags, event->data);
        }
      }
    }
    qtimer_process(&(engine->timer_mng));
  }
  return 0;
}

void qengine_destroy(qengine_t *engine) {
  qfree(engine);
}

qid_t qengine_add_timer(qengine_t* engine, uint32_t timeout_ms, qtimer_func_t *func, int type, void *data) {
  return qtimer_add(&engine->timer_mng, timeout_ms, func, type, data);
}

int qengine_del_timer(qengine_t* engine, qid_t id) {
  return qtimer_del(&engine->timer_mng, id);
}
