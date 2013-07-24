/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QFREELIST_H__
#define __QFREELIST_H__

#include "qlist.h"

#define qfree_item_fields \
  qlist_t fentry

typedef struct qfree_item_t {
  qfree_item_fields;
} qfree_item_t;

typedef int  (*qitem_init_pt)(void *);
typedef void (*qitem_destroy_pt)(void *);

/* not thread-safe, if use in multithread MUST lock outside */
typedef struct qfreelist_t { 
  qlist_t           free;
  const char       *name;
  int               size;
  int               initnum;
  qitem_init_pt     init;
  qitem_destroy_pt  destroy;
} qfreelist_t;

int   qfreelist_init(qfreelist_t *flist, const char *name,
                     int size, int num, qitem_init_pt init,
                     qitem_destroy_pt destroy);
void  qfreelist_destroy(qfreelist_t *flist);
void* qfreelist_alloc(qfreelist_t *flist);

static inline void
qfreelist_free(qfreelist_t *flist, qfree_item_t *item) {
  qlist_add_tail(&(item->fentry), &(flist->free));
}

#endif  /* __QFREELIST_H__ */
