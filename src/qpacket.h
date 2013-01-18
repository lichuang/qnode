/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QPACKET_H__
#define __QPACKET_H__

#include "qlist.h"

#define QBUFFER_SIZE 1000

typedef struct qbuffer_t {
  qlist_t entry;
  char data[QBUFFER_SIZE];
  size_t pos;
  size_t size;
} qbuffer_t;

typedef struct qpacket_t {
  qlist_t list;       /* buffer list */
  qlist_t *head;      /* buffer list head */
  qlist_t *current;   /* current buffer list pointer */
  size_t size;
} qpacket_t;

void qpacket_init(qpacket_t *packet);
void qpacket_destroy(qpacket_t *packet);

#endif  /* __QPACKET_H__ */
