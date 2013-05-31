/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QSIGNAL_H__
#define __QSIGNAL_H__

#include "qatomic.h"
#include "qcore.h"

struct qsignal_t {
  int         rfd;
  int         wfd;
  qatomic_t   active;
  qmailbox_t *box;
};

#define qsignal_get_fd(signal)  \
  (signal)->rfd

void        qsignal_send(qsignal_t *channel);
void        qsignal_recv(qsignal_t *channel);
int         qsignal_active(qsignal_t *channel, int active);

void        qsignal_init(qsignal_t *, qmailbox_t *);

#endif /* __QSIGNAL_H__ */
