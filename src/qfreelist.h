/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QFREELIST_H__
#define __QFREELIST_H__

#include "qlist.h"

#define qfreeitem_fields \
  qlist_t fentry

typedef struct qfreeitem_t {
  qfreeitem_fields;
} qfreeitem_t;

typedef int  (*qfreeitem_ctor_pt)(void *);
typedef void (*qfreeitem_dtor_pt)(void *);

typedef struct qfreelist_conf_t {
  const char         *name;
  int                 size;
  int                 num;
  qfreeitem_ctor_pt   ctor;
  qfreeitem_dtor_pt   dtor;
} qfreelist_conf_t;

#define QFREELIST_CONF(str, s, n, c, d) \
  {.name = (str), .size = (s), .num = (n), .ctor = (c), .dtor = (d)}

/* not thread-safe, if use in multithread MUST lock outside */
typedef struct qfreelist_t { 
  /* alloc item list */
  qlist_t             alloc;

  /* free item list */
  qlist_t             free;

  /* name of the free list */
  const char         *name;

  /* size of the per free item */
  int                 size;

  /* free list init num */
  int                 initnum;

  /* called when item allocate */
  qfreeitem_ctor_pt   ctor;

  /* called when item free */
  qfreeitem_dtor_pt   dtor;
} qfreelist_t;

int   qfreelist_init(qfreelist_t *flist, qfreelist_conf_t *conf);
void  qfreelist_destroy(qfreelist_t *flist);

void* qfreelist_new(qfreelist_t *flist);
void  qfreelist_free(qfreelist_t *flist, qfreeitem_t *item);

#endif  /* __QFREELIST_H__ */
