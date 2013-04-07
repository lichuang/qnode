/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QTIMER_H__
#define __QTIMER_H__

#include <stdint.h>
#include "qcore.h"
#include "qidmap.h"
#include "qminheap.h"
#include "qlist.h"
#include "qtype.h"

typedef struct qtimer_t {
  int             heap_index;   /* heap index in the minheap */
  void           *arg;          /* client data */
  uint64_t        timeout;      /* timeout(in ms) */
  uint64_t        cycle;        /* timer cycle */
  qid_t           id;           /* timer id */
  qlist_t         entry;        /* timer free list entry */
  qtimer_func_t  *handler;      /* timer handler */
  char data[1000];
} qtimer_t;

typedef struct qtimer_manager_t {
  uint64_t        now;
  uint64_t        now_ms;
  qidmap_t        id_map;
  qlist_t         free_list;    /* free timer list */
  qminheap_t      min_heap;
  qengine_t      *engine;
} qtimer_manager_t;

void  qtimer_manager_init(qtimer_manager_t *mng, qengine_t *engine);
void  qtimer_manager_free(qtimer_manager_t *mng);
qid_t qtimer_add(qtimer_manager_t *mng, uint32_t timeout,
                 qtimer_func_t *func, uint32_t cycle, void *arg);
int   qtimer_del(qtimer_manager_t *mng, qid_t id);
int   qtimer_next(qtimer_manager_t *mng);
void  qtimer_process(qtimer_manager_t *mng);

#endif  /* __QTIMER_H__ */
