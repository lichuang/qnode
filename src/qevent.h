/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QEVENT_H__
#define __QEVENT_H__

#include "qcore.h"
#include "qlist.h"

typedef void (qevent_pt)(int fd, int flags, void *data);

struct qevent_t {
  int            fd;

  qlist_t        active_entry;

  int            events;

  /* active events flags */
  int            flags;

  unsigned       error:1;
  qevent_pt     *read;
  qevent_pt     *write;
  void          *data;
};

void qevent_init(qevent_t *event, int fd,
                 qevent_pt *read, qevent_pt *write,
                 void *data);

#endif  /* __QEVENT_H__ */
