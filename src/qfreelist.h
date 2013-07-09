/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QFREELIST_H__
#define __QFREELIST_H__

#include "qlist.h"

#define qfree_item_fields \
  qlist_t free

typedef struct qfree_item_t {
  qfree_item_fields;
} qfree_item_t;

/* not thread-safe, if use in multithread MUST lock outside */
typedef struct qfreelist_t { 
  qlist_t     free;
  const char *name;
  int         size;
  int         initnum;
} qfreelist_t;

void  qfreelist_init(qfreelist_t *flist, const char *name,
                     int size, int num);
void  qfreelist_destroy(qfreelist_t *flist);
void* qfreelist_alloc(qfreelist_t *flist);
void  qfreelist_free(qfreelist_t *flist, qfree_item_t *item);

#endif  /* __QFREELIST_H__ */
