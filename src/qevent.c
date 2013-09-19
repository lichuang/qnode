/*
 * See Copyright Notice in qnode.h
 */

#include "qevent.h"

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
