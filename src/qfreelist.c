/*
 * See Copyright Notice in qnode.h
 */

#include "qalloc.h"
#include "qassert.h"
#include "qcore.h"
#include "qlog.h"
#include "qfreelist.h"

static int  prealloc(qfreelist_t *flist);
static void destroy(qlist_t *list, qfreeitem_final_pt final);

int
qfreelist_init(qfreelist_t *flist, qfreelist_conf_t *conf) {
  qlist_entry_init(&(flist->free));
  qlist_entry_init(&(flist->alloc));
  flist->size     = conf->size;
  flist->name     = conf->name;
  flist->initnum  = conf->num;
  flist->ctor     = conf->ctor;
  flist->dtor     = conf->dtor;
  flist->final    = conf->final;

  return prealloc(flist);
}

static int
prealloc(qfreelist_t *flist) {
  int           i;
  int           num, size;
  qfreeitem_t *item;

  qassert(qlist_empty(&(flist->free)));
  num  = flist->initnum;
  size = flist->size;
  for (i = num; i > 0; --i) {
    item = (qfreeitem_t*)qcalloc(size);
    if (item == NULL) {
      return QERROR;
    }
    qlist_add_tail(&(item->fentry), &(flist->free));
  }

  return QOK;
}

static void
destroy(qlist_t *list, qfreeitem_final_pt final) {
  qfreeitem_t *item;
  qlist_t      *pos;

  for (pos = list->next; pos != list; ) {
    item = qlist_entry(pos, qfreeitem_t, fentry);
    pos = pos->next;
    qlist_del(&(item->fentry));
    if (final) {
      final(item);
    }
    qfree(item);
  }
  qassert(qlist_empty(list));
}

void
qfreelist_destroy(qfreelist_t *flist) {
  destroy(&(flist->free),  flist->final);
  destroy(&(flist->alloc), flist->final);
}

void*
qfreelist_new(qfreelist_t *flist) {
  qfreeitem_t *item;
  qlist_t      *pos;

  if (qlist_empty(&(flist->free))) {
    prealloc(flist);
    if (qlist_empty(&(flist->free))) {
      return NULL;
    }
  }
  pos = flist->free.next;
  item = qlist_entry(pos, qfreeitem_t, fentry);    
  if (flist->ctor && flist->ctor(item) != QOK) {
    return NULL;
  }
  qlist_del_init(&item->fentry);
  qlist_add_tail(&item->fentry, &flist->alloc);
  if (item->flag == 0) {
    item->flag = 1;
  }
  item->active = 1;

  return item;
}

void
qfreelist_free(qfreelist_t *flist, qfreeitem_t *item) {
  if (flist->dtor) {
    flist->dtor(item);
  }
  qlist_del_init(&item->fentry);
  qlist_add_tail(&(item->fentry), &(flist->free));
  item->active = 0;
}

