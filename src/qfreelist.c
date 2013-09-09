/*
 * See Copyright Notice in qnode.h
 */

#include "qalloc.h"
#include "qassert.h"
#include "qcore.h"
#include "qfreelist.h"

static int prealloc(qfreelist_t *flist, qitem_init_pt init);

int
qfreelist_init(qfreelist_t *flist, const char *name,
               int size, int num,
               qitem_init_pt init, qitem_destroy_pt destroy) {
  qlist_entry_init(&(flist->free));
  qlist_entry_init(&(flist->alloc));
  flist->size     = size;
  flist->name     = name;
  flist->initnum  = num;
  flist->init     = init;
  flist->destroy  = destroy;

  return prealloc(flist, init);
}

static int
prealloc(qfreelist_t *flist, qitem_init_pt init) {
  int           i;
  int           num, size;
  qfree_item_t *item;

  qassert(qlist_empty(&(flist->free)));
  num  = flist->initnum;
  size = flist->size;
  for (i = num; i > 0; --i) {
    item = (qfree_item_t*)qcalloc(size);
    if (item == NULL) {
      return QERROR;
    }
    if (init) {
      init(item);
    }
    qlist_add_tail(&(item->fentry), &(flist->free));
  }

  return QOK;
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
    prealloc(flist, NULL);
    if (qlist_empty(&(flist->free))) {
      return NULL;
    }
  }
  pos = flist->free.next;
  item = qlist_entry(pos, qfree_item_t, fentry);    
  qlist_del_init(&item->fentry);
  qlist_add_tail(&item->fentry, &flist->alloc);

  return item;
}
