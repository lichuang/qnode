/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QEVENT_H__
#define __QEVENT_H__

typedef void (qevent_pt)(int fd, int flags, void *data);

struct qevent_t {
  int            fd;
  int            flags;
  qevent_pt     *read;
  qevent_pt     *write;
  void          *data;
};

#endif  /* __QEVENT_H__ */
