/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QTIMERHEAP_H__
#define __QTIMERHEAP_H__

#include <sys/time.h>
#include "qidmap.h"
#include "qtype.h"

enum {
  QTIMER_ONCE,
  QTIMER_REPEAT,
};

typedef struct qtimer_t {
  int type;
  qtimer_func_t *callback;
  int min_heap_idx;
  void *data;
  struct timeval timeout;
  qid_t id;
} qtimer_t;

typedef struct qtimer_heap_t {
  struct qtimer_t *timers[QID_MAX];
  struct qtimer_t *timer_heap[QID_MAX];
  qidmap_t id_map;
  unsigned int size;
} qtimer_heap_t;

void  qtimer_heap_init(qtimer_heap_t *heap);
qid_t qtimer_add(qtimer_heap_t *heap, struct timeval timeout, qtimer_func_t *func, int type, void *data);
int   qtimer_del(qtimer_heap_t *heap, qid_t id);
long  qtimer_nearest(qtimer_heap_t *heap);
void  qtimer_process(qtimer_heap_t *heap);

#endif  /* __QTIMERHEAP_H__ */
