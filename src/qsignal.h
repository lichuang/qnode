/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QSIGNAL_H__
#define __QSIGNAL_H__

#include "qatomic.h"

typedef struct qsignal_t {
  int rfd;
  int wfd;
  qatomic_t active;        /* active if there is mail */
} qsignal_t;

qsignal_t* qsignal_new();
void qsignal_destroy(qsignal_t *signal);
int qsignal_get_fd(qsignal_t *signal);
void qsignal_send(qsignal_t *signal);
void qsignal_recv(qsignal_t *signal);

int qsignal_active(qsignal_t *signal, int active);

#endif /* __QSIGNAL_H__ */
