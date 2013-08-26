/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qassert.h"
#include "qcore.h"
#include "qidmap.h"

/* page size = 2^12 = 4K */
#define PAGE_SHIFT    12
#define PAGE_SIZE    (1UL << PAGE_SHIFT)

#define BITS_PER_BYTE        8
#define BITS_PER_PAGE        (PAGE_SIZE * BITS_PER_BYTE)
#define BITS_PER_PAGE_MASK    (BITS_PER_PAGE - 1)

static int  alloc_qid(qidmap_t *idmap);

static int
alloc_qid(qidmap_t *idmap) {
  int qid;

  qid = idmap->current;
  do {
    if (idmap->data[qid] == NULL) {
      idmap->current = (qid + 1) % QID_MAX;
      return qid;
    }
    qid = (qid + 1) % QID_MAX;
  } while(qid != idmap->current);

  return (int)QINVALID_ID;
}

void
qid_detach(qidmap_t *idmap, qid_t id) {
  idmap->data[id] = NULL;
}

void
qidmap_init(qidmap_t *idmap) {
  int i;

  idmap->current = 0;
  for (i = 0; i < QID_MAX; ++i) {
    idmap->data[i] = NULL;
  }
}

qid_t
qid_new(qidmap_t *idmap) {
  return alloc_qid(idmap);
}

void
qid_attach(qidmap_t *idmap, qid_t id, void *data) {
  qassert(idmap->data[id] == NULL);

  idmap->data[id] = data;
}
