/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QMSG_H__
#define __QMSG_H__

#include "qactor.h"
#include "qlist.h"

/* define messages between worker-thread and main-thread */
typedef struct qmsg_t {
  qlist_t entry;

  enum {
    start = 0,
  } type;

  union {
    struct {
      struct qactor_t *actor;
    } start;
  } args;
} qmsg_t;

qmsg_t* qmsg_new();

#define qmsg_init_start(msg, actor)               \
do {                                              \
  qlist_entry_init(&(msg->entry));                \
  msg->type = start;                              \
  msg->args.start.actor = (actor);                \
} while (0)

#endif  /* __QMSG_H__ */
