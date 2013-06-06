/*
 * See Copyright Notice in qnode.h
 */

#include "qmsg.h"
#include "qmmsg.h"

/* W_* means Worker thread handle message */
enum {
  W_START   = 0,
  W_SPAWN   = 1,
  QWMSG_NUM
};

typedef struct qwmsg_start_t {
  qmsg_header_fields;
  qid_t aid;
} qwmsg_start_t;

typedef qmmsg_spawn_t qwmsg_spawn_t;

qmsg_t* qwmsg_start_new(qid_t aid, qid_t sender, qid_t recver);
