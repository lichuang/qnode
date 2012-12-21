/*
 * See Copyright Notice in qnode.h
 */

#include "qevent.h"
#include "qmalloc.h"
#include "qminheap.h"

extern int is_time_small(const struct timeval *time1, const struct timeval *time2);

int min_heap_elem_greater(struct qnode_event_t *event1, struct qnode_event_t *event2) {
      return is_time_small(&(event2->timeout), &(event1->timeout));
}

void qnode_minheap_ctor(qnode_minheap_t* heap) {
  heap->events = NULL;
  heap->size = 0;
  heap->add = 0;
}

void qnode_minheap_dtor(qnode_minheap_t* heap) {
  if(heap->events) {
    qnode_free(heap->events);
  }
}

void qnode_minheap_elem_init(struct qnode_event_t* e) {
  e->min_heap_idx = -1;
}

int qnode_minheap_empty(qnode_minheap_t* heap) {
  return 0u == heap->size;
}

unsigned int qnode_minheap_size(qnode_minheap_t* heap) {
  return heap->size;
}

struct qnode_event_t* qnode_minheap_top(qnode_minheap_t* heap) {
  return heap->size ? *heap->events : NULL;
}

int qnode_minheap_reserve(qnode_minheap_t* heap, unsigned int size) {
  if(heap->add < size) {
    struct qnode_event_t** events;
    unsigned add = heap->add ? heap->add * 2 : 8;
    if(add < size)
      add = size;
    events = (struct qnode_event_t**)qnode_realloc(heap->events, add * sizeof (*events));
    if(!events) {
      return -1;
    }
    heap->events = events;
    heap->add = add;
  }
  return 0;
}

void min_heap_shift_up(qnode_minheap_t* heap, unsigned hole_index, struct qnode_event_t* e) {
  unsigned parent = (hole_index - 1) / 2;
  while(hole_index && min_heap_elem_greater(heap->events[parent], e)) {
    (heap->events[hole_index] = heap->events[parent])->min_heap_idx = hole_index;
    hole_index = parent;
    parent = (hole_index - 1) / 2;
  }
  (heap->events[hole_index] = e)->min_heap_idx = hole_index;
}

void min_heap_shift_down(qnode_minheap_t* heap, unsigned hole_index, struct qnode_event_t* e) {
  unsigned min_child = 2 * (hole_index + 1);
  while(min_child <= heap->size) {
    min_child -= min_child == heap->size || min_heap_elem_greater(heap->events[min_child], heap->events[min_child - 1]);
    if(!(min_heap_elem_greater(e, heap->events[min_child])))
      break;
    (heap->events[hole_index] = heap->events[min_child])->min_heap_idx = hole_index;
    hole_index = min_child;
    min_child = 2 * (hole_index + 1);
  }
  min_heap_shift_up(heap, hole_index,  e);
}


int qnode_minheap_push(qnode_minheap_t* heap, struct qnode_event_t* e) {
  if(qnode_minheap_reserve(heap, heap->size + 1)) {
    return -1;
  }
  min_heap_shift_up(heap, heap->size++, e);
  return 0;
}

struct qnode_event_t* qnode_minheap_pop(qnode_minheap_t* heap) {
  if(heap->size) {
    struct qnode_event_t* e = *heap->events;
    min_heap_shift_down(heap, 0u, heap->events[--heap->size]);
    e->min_heap_idx = -1;
    return e;
  }
  return 0;
}

int qnode_minheap_erase(qnode_minheap_t* heap, struct qnode_event_t* e) {
  if(((unsigned int)-1) != e->min_heap_idx) {
    struct qnode_event_t *last = heap->events[--heap->size];
    unsigned parent = (e->min_heap_idx - 1) / 2;
    if (e->min_heap_idx > 0 && min_heap_elem_greater(heap->events[parent], last))
      min_heap_shift_up(heap, e->min_heap_idx, last);
    else
      min_heap_shift_down(heap, e->min_heap_idx, last);
    e->min_heap_idx = -1;
    return 0;
  }
  return -1;
}
