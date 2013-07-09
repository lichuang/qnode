/*
 * See Copyright Notice in qnode.h
 */

#include "qalloc.h"
#include "qassert.h"
#include "qfreelist.h"

static void prealloc(qfreelist_t *flist);

void
qfreelist_init(qfreelist_t *flist, const char *name, int size, int num) {
  qlist_entry_init(&(flist->free));
  flist->size = size;
  flist->name = name;
  flist->initnum = num;
  prealloc(flist);
}

static void
prealloc(qfreelist_t *flist) {
  int           i;
  int           num, size;
  qfree_item_t *item;

  qassert(qlist_empty(&(flist->free)));
  num  = flist->initnum;
  size = flist->size;
  for (i = num; i > 0; --i) {
    item = (qfree_item_t*)qcalloc(size);
    if (item == NULL) {
      break;
    }
    qlist_add_tail(&(item->free), &(flist->free));
  }
}

void
qfreelist_destroy(qfreelist_t *flist) {
  qfree_item_t *item;
  qlist_t      *pos, *list;

  list = &(flist->free);
  for (pos = list->next; pos != list; ) {
    item = qlist_entry(pos, qfree_item_t, free);
    pos = pos->next;
    qlist_del(&(item->free));
    qfree(item);
  }
}

void*
qfreelist_alloc(qfreelist_t *flist) {
  qfree_item_t *item;
  qlist_t      *pos;

  if (qlist_empty(&(flist->free))) {
    prealloc(flist);
    if (qlist_empty(&(flist->free))) {
      return NULL;
    }
  }
  pos = flist->free.next;
  item = qlist_entry(pos, qfree_item_t, free);    
  qlist_del_init(&item->free);

  return item;
}

void
qfreelist_free(qfreelist_t *flist, qfree_item_t *item) {
  qlist_add_tail(&(item->free), &(flist->free));
}
