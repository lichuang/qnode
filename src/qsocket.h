/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QSOCKET_H__
#define __QSOCKET_H__

#include "qlist.h"
#include "qpacket.h"
#include "qtype.h"

struct qengine_t;

typedef struct qsocket_t {
  qlist_t entry;
  int fd;
  qid_t aid;
  qpacket_t packet;
  struct qengine_t *engine;
} qsocket_t;

qsocket_t* qsocket_get(int fd);

#endif  /* __QSOCKET_H__ */
