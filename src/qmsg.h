/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QMSG_H__
#define __QMSG_H__

#include "qcore.h"
#include "qdict.h"
#include "qlist.h"
#include "qstring.h"
#include "qtype.h"

#define QINVALID_MSG  -1

typedef void (*qmsg_destroy_t)(qmsg_t *);

#define qmsg_header_fields    \
  qlist_t         entry;      \
  int             type;       \
  int             size;       \
  qid_t           sender;     \
  qid_t           recver;     \
  qmsg_destroy_t  destroy

struct qmsg_t {
  qmsg_header_fields;
};

qmsg_t* qmsg_new(qid_t sender, qid_t recver, int size, int type);
void    qmsg_destroy(qmsg_t *msg);
qmsg_t* qmsg_clone(qmsg_t *msg);
void    qmsg_send(qmsg_t *msg);

#endif  /* __QMSG_H__ */
