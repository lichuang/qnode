/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QENGINE_H__
#define __QENGINE_H__

#include <stdint.h>
#include <sys/time.h>
#include "qcore.h"
#include "qevent.h"
#include "qlist.h"
#include "qtimer.h"

#define QINIT_EVENTS   1024

/* for events */
#define QEVENT_READ   1 << 0
#define QEVENT_WRITE  1 << 1 
#define QEVENT_ERROR  1 << 2  

struct qengine_t;
struct qevent_t;
struct qdispatcher_t;

typedef struct qdispatcher_t {
  const char *name;
  int  (*init)(qengine_t*);
  int  (*add)(qengine_t *engine, int fd, int flags);
  int  (*del)(qengine_t *engine, int fd, int flags);
  int  (*poll)(qengine_t *, int timeout_ms);
  int  (*expand)(qengine_t *);
  void (*destroy)(qengine_t *);
} qdispatcher_t;

struct qengine_t {
  unsigned int          quit:1;
  qevent_t            **events;
  qlist_t               active;
  const qdispatcher_t  *dispatcher;
  void                 *data;
  int                   max_fd;
  int                   size;
  qtimer_manager_t      timer_mng;
};

qengine_t* qengine_new();
qengine_t* qengine_expand(qengine_t *engine);
int        qengine_loop(qengine_t* engine);
void       qengine_destroy(qengine_t *engine);

#endif  /* __QENGINE_H__ */
