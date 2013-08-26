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

typedef void (qtimer_pt)(void *data);

typedef struct qtimer_t {
  /* heap index in the minheap */
  int             heap_index;

  /* client data */
  void           *arg;

  /* timeout(in ms) */
  uint64_t        timeout;

  /* timer cycle(in ms) */
  uint64_t        cycle;

  /* timer id */
  qid_t           id;

  /* timer free list entry */
  qlist_t         entry;

  /* timer handler */
  qtimer_pt      *handler;      /* timer handler */
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
                 qtimer_pt *func, uint32_t cycle, void *arg);
int   qtimer_del(qtimer_manager_t *mng, qid_t id);
int   qtimer_next(qtimer_manager_t *mng);
void  qtimer_process(qtimer_manager_t *mng);

#endif  /* __QTIMER_H__ */
