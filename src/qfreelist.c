/*
 * See Copyright Notice in qnode.h
 */

#include "qalloc.h"
#include "qassert.h"
#include "qcore.h"
#include "qfreelist.h"

static int  prealloc(qfreelist_t *flist);
static void destroy(qlist_t *list);

int
qfreelist_init(qfreelist_t *flist, const char *name,
               int size, int num,
               qitem_ctor_pt ctor, qitem_dtor_pt dtor) {
  qlist_entry_init(&(flist->free));
  qlist_entry_init(&(flist->alloc));
  flist->size     = size;
  flist->name     = name;
  flist->initnum  = num;
  flist->ctor     = ctor;
  flist->dtor     = dtor;

  return prealloc(flist);
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
      return QERROR;
    }
    qlist_add_tail(&(item->fentry), &(flist->free));
  }

  return QOK;
}

static void
destroy(qlist_t *list) {
  qfree_item_t *item;
  qlist_t      *pos;

  for (pos = list->next; pos != list; ) {
    item = qlist_entry(pos, qfree_item_t, fentry);
    pos = pos->next;
    qlist_del(&(item->fentry));
    qfree(item);
  }
}

void
qfreelist_destroy(qfreelist_t *flist) {
  destroy(&(flist->free));
  destroy(&(flist->alloc));
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
  qlist_add_tail(&item->fentry, &flist->alloc);
  if (flist->ctor) {
    flist->ctor(item);
  }

  return item;
}

void
qfreelist_free(qfreelist_t *flist, qfree_item_t *item) {
  if (flist->dtor) {
    flist->dtor(item);
  }
  qlist_del_init(&item->fentry);
  qlist_add_tail(&(item->fentry), &(flist->free));
}

