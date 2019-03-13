/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QIDMAP_H__
#define __QIDMAP_H__

#include "qtype.h"

#define QID_MAX 10240

typedef struct qidmap_t {
  qid_t         current;
  void*         data[QID_MAX];
} qidmap_t;

void   qidmap_init(qidmap_t *idmap);
qid_t  qid_new(qidmap_t *idmap);
void   qid_detach(qidmap_t *idmap, qid_t qid);
void   qid_attach(qidmap_t *idmap, qid_t id, void *data);

#endif  /* __QIDMAP_H__ */
