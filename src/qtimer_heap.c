/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qmalloc.h"
#include "qtimer_heap.h"

static int is_time_greater(const struct timeval *time1, const struct timeval *time2) {
  return time1->tv_sec == time2->tv_sec ?
          time1->tv_usec > time2->tv_usec :
          time1->tv_sec  > time2->tv_sec;
}

static void calc_timeout(struct timeval *timeout, qtime_t timeout_ms) {
  qtime_t sec, ms;
  gettimeofday(timeout, NULL);
  sec = timeout_ms / 1000;
  ms  = timeout_ms % 1000;
  if (ms >= 1000) {
    sec++;
    ms -= 1000;
  }

  timeout->tv_sec  += sec;
  timeout->tv_usec += ms * 1000;
}

static inline int min_heap_elem_greater(qtimer_t *timer1, qtimer_t *timer2) {
  struct timeval *time1 = &(timer1->timeout);
  struct timeval *time2 = &(timer2->timeout);
  return is_time_greater(time1, time2);
}

static void min_heap_shift_up(qtimer_heap_t* heap, unsigned hole_index, qtimer_t* timer) {
  unsigned parent = (hole_index - 1) / 2;
  while(hole_index && min_heap_elem_greater(heap->timer_heap[parent], timer)) {
    heap->timer_heap[hole_index] = heap->timer_heap[parent];
    heap->timer_heap[parent]->min_heap_idx = hole_index;
    hole_index = parent;
    parent = (hole_index - 1) / 2;
  }
  heap->timer_heap[hole_index] = timer;
  timer->min_heap_idx = hole_index;
}

static void min_heap_shift_down(qtimer_heap_t* heap, unsigned hole_index, qtimer_t* timer) {
  unsigned min_child = 2 * (hole_index + 1);
  while(min_child <= heap->size) {
    min_child -= min_child == heap->size || min_heap_elem_greater(heap->timer_heap[min_child], heap->timer_heap[min_child - 1]);
    if(!(min_heap_elem_greater(timer, heap->timer_heap[min_child]))) {
      break;
    }
    heap->timer_heap[hole_index] = heap->timer_heap[min_child];
    heap->timer_heap[min_child]->min_heap_idx = hole_index;
    hole_index = min_child;
    min_child = 2 * (hole_index + 1);
  }
  min_heap_shift_up(heap, hole_index, timer);
}

static int min_heap_erase(qtimer_heap_t* heap, qtimer_t* timer) {
  if(timer->min_heap_idx != -1) {
    qtimer_t *last = heap->timer_heap[--heap->size];
    unsigned parent = (timer->min_heap_idx - 1) / 2;
    if (timer->min_heap_idx > 0 && min_heap_elem_greater(heap->timer_heap[parent], last)) {
      min_heap_shift_up(heap, timer->min_heap_idx, last);
    } else {
      min_heap_shift_down(heap, timer->min_heap_idx, last);
    }
    timer->min_heap_idx = -1;
    return 0;
  }
  return -1;
}

void  qtimer_heap_init(qtimer_heap_t *heap) {
  int i = 0;
  for (i = 0; i < QID_MAX; ++i) {
    heap->timers[i] = NULL;
    heap->timer_heap[i] = NULL;
  }
  qidmap_init(&heap->id_map);
  heap->size = heap->num = 0;
}

static void min_heap_push(qtimer_heap_t *heap, qtimer_t *timer) {
  calc_timeout(&timer->timeout, timer->ms);
  heap->num += 1;
  min_heap_shift_up(heap, heap->size++, timer);
}

qid_t qtimer_add(qtimer_heap_t *heap, qtime_t timeout_ms, qtimer_func_t *func, int type, void *data) {
  qid_t id = qid_new(&heap->id_map);
  if (id == QID_INVALID) {
    return QID_INVALID;
  }
  qtimer_t *timer = heap->timers[id];
  if (timer == NULL) {
    timer = qalloc_type(qtimer_t);
    if (timer == NULL) {
      return QID_INVALID;
    }
  }
  timer->type = type;
  timer->callback = func;
  timer->min_heap_idx = -1;
  timer->id = id;
  timer->data = data;
  timer->ms = timeout_ms;
  min_heap_push(heap, timer);
  return id;
}

int qtimer_del(qtimer_heap_t *heap, qid_t id) {
  qtimer_t *timer = heap->timers[id];
  if (timer == NULL) {
    return -1;
  }
  heap->num -= 1;
  qid_free(&heap->id_map, id);
  int ret = min_heap_erase(heap, timer);
  if (timer->type == QTIMER_ONCE) {
    qfree(timer);
  }
  return ret;
}

struct timeval* qtimer_nearest(qtimer_heap_t *heap) {
  if (heap->num == 0) {
    return NULL;
  }
  qtimer_t *timer = heap->timer_heap[0];
  return &(timer->timeout);
}

static inline qtimer_t* min_heap_top(qtimer_heap_t *heap) {
  if (heap->num == 0) {
    return NULL;
  }
  return heap->timer_heap[0];
}

void qtimer_process(qtimer_heap_t *heap, const struct timeval *time) {
  qtimer_t *timer;
  while ((timer = min_heap_top(heap)) != NULL) {
    if (is_time_greater(&timer->timeout, time)) {
      break;
    }

    timer->callback(timer->data);
    qtimer_del(heap, timer->id);
    if (timer->type == QTIMER_REPEAT) {
      min_heap_push(heap, timer);
    }
  }
}
