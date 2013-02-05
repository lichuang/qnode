/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QTIMER_H__
#define __QTIMER_H__

#include <stdint.h>
#include "qidmap.h"
#include "qminheap.h"
#include "qlist.h"
#include "qtype.h"

struct qengine_t;

typedef struct qtimer_t {
  qtimer_func_t*  handler;
  void*           arg;
  uint64_t        timeout;
  uint64_t        cycle;
  qid_t           id;
  qlist_t         entry;
  int             heap_index;
} qtimer_t;

typedef struct qtimer_manager_t {
  struct qengine_t* engine;
  qidmap_t          id_map;
  qlist_t           free_list;
  qminheap_t        min_heap;
} qtimer_manager_t;

void  qtimer_manager_init(qtimer_manager_t *mng, struct qengine_t *engine);
qid_t qtimer_add(qtimer_manager_t *mng, uint32_t timeout,
                 qtimer_func_t *func, uint32_t cycle, void *arg);
int   qtimer_del(qtimer_manager_t *mng, qid_t id);
int   qtimer_next(qtimer_manager_t *mng);
void  qtimer_process(qtimer_manager_t *mng);

#endif  /* __QTIMER_H__ */
