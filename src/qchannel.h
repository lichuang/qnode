/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QSIGNAL_H__
#define __QSIGNAL_H__

#include "qatomic.h"
#include "qcore.h"

struct qchannel_t {
  int       rfd;
  int       wfd;
  qatomic_t active;
};

qchannel_t* qchannel_new();
int qchannel_get_fd(qchannel_t *channel);
void qchannel_send(qchannel_t *channel);
void qchannel_recv(qchannel_t *channel);

int qchannel_active(qchannel_t *channel, int active);

#endif /* __QSIGNAL_H__ */
