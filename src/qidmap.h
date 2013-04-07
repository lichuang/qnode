/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QIDMAP_H__
#define __QIDMAP_H__

#include "qtype.h"

/* id allocate algorithm, refer to Linux Kernel pid map */

/* max id, equal to 2^15=32768 */
#define QID_MAX 0x8000

#define QID_INVALID  -1

typedef struct qidmap_t {
  qid_t         last_qid;
  unsigned int  nr_free;
  char          page[QID_MAX];
  void*         data[QID_MAX];
} qidmap_t;

void   qidmap_init(qidmap_t *idmap);
qid_t  qid_new(qidmap_t *idmap);
void   qid_free(qidmap_t *idmap, qid_t qid);
void   qid_attach(qidmap_t *idmap, qid_t id, void *data);
void   qid_detach(qidmap_t *idmap, qid_t id);

#endif  /* __QIDMAP_H__ */
