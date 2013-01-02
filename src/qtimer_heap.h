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
  qtime_t ms;
  qid_t id;
} qtimer_t;

typedef struct qtimer_heap_t {
  struct qtimer_t *timers[QID_MAX];
  struct qtimer_t *timer_heap[QID_MAX];
  qidmap_t id_map;
  unsigned int size;
  unsigned int num;
} qtimer_heap_t;

void    qtimer_heap_init(qtimer_heap_t *heap);
qid_t   qtimer_add(qtimer_heap_t *heap, qtime_t timeout_ms, qtimer_func_t *func, int type, void *data);
int     qtimer_del(qtimer_heap_t *heap, qid_t id);
struct timeval* qtimer_nearest(qtimer_heap_t *heap);
void    qtimer_process(qtimer_heap_t *heap, const struct timeval *time);

#endif  /* __QTIMERHEAP_H__ */
