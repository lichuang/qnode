/*
 * See Copyright Notice in qnode.h
 */

#include "qalloc.h"
#include "qassert.h"
#include "qfreelist.h"

static int prealloc(qfreelist_t *flist);

int
qfreelist_init(qfreelist_t *flist, const char *name,
               int size, int num, qitem_init_pt init,
               qitem_destroy_pt destroy) {
  int ret;

  qlist_entry_init(&(flist->free));
  flist->size     = size;
  flist->name     = name;
  flist->initnum  = num;
  flist->init     = init;
  flist->destroy  = destroy;
  ret = prealloc(flist);

  return ret;
}

static int
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
      return -1;
    }
    if (flist->init) {
      flist->init(item);
    }
    qlist_add_tail(&(item->fentry), &(flist->free));
  }

  return 0;
}

void
qfreelist_destroy(qfreelist_t *flist) {
  qfree_item_t *item;
  qlist_t      *pos, *list;

  list = &(flist->free);
  for (pos = list->next; pos != list; ) {
    item = qlist_entry(pos, qfree_item_t, fentry);
    pos = pos->next;
    qlist_del(&(item->fentry));
    if (flist->destroy) {
      flist->destroy(item);
    }
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
  item = qlist_entry(pos, qfree_item_t, fentry);    
  qlist_del_init(&item->fentry);

  return item;
}
