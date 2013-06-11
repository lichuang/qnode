/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QLMSG_H__
#define __QLMSG_H__

#include "qcore.h"
#include "qmsg.h"

/* L_* means Logger thread handle message */
enum {
  L_LOG     = 0,
  L_SIGNAL  = 1,
  QLMSG_NUM
};

typedef struct qlmsg_log_t {
  qmsg_header_fields;

  qlog_t  *log;
} qlmsg_log_t;

typedef struct qlmsg_signal_t {
  qmsg_header_fields;

  int   signo;
} qlmsg_signal_t;

qmsg_t* qlmsg_log_new(qlog_t *log, int sender);
qmsg_t* qlmsg_signal_new(int signo);

#endif  /* __QLMSG_H__ */
