/*
 * See Copyright Notice in qnode.h
 */

#include "qevent.h"
#include "qmin_heap.h"

int min_heap_elem_greater(struct qnode_event_t *a, struct qnode_event_t *b)
{
      return evutil_timercmp(&a->ev_timeout, &b->ev_timeout, >);
}

void qnode_min_heap_ctor(qnode_min_heap_t* s) {
  s->p = 0;
  s->n = 0;
  s->a = 0;
}

void qnode_min_heap_dtor(qnode_min_heap_t* s) {
  if(s->p) {
    free(s->p);
  }
}

void qnode_min_heap_elem_init(struct qnode_event_t* e) {
  e->min_heap_idx = -1;
}

int qnode_min_heap_empty(qnode_min_heap_t* s) {
  return 0u == s->n;
}

unsigned qnode_min_heap_size(qnode_min_heap_t* s) {
  return s->n;
}

struct qnode_event_t* qnode_min_heap_top(qnode_min_heap_t* s) {
  return s->n ? *s->p : NULL;
}

int qnode_min_heap_reserve(qnode_min_heap_t* s, unsigned n) {
  if(s->a < n) {
    struct qnode_event_t** p;
    unsigned a = s->a ? s->a * 2 : 8;
    if(a < n)
      a = n;
    p = (struct qnode_t**)qrealloc(s->p, a * sizeof (*p));
    if(!p) {
      return -1;
    }
    s->p = p;
    s->a = a;
  }
  return 0;
}

void min_heap_shift_up(qnode_min_heap_t* s, unsigned hole_index, struct qnode_event_t* e) {
  unsigned parent = (hole_index - 1) / 2;
  while(hole_index && qnode_min_heap_elem_greater(s->p[parent], e)) {
    (s->p[hole_index] = s->p[parent])->min_heap_idx = hole_index;
    hole_index = parent;
    parent = (hole_index - 1) / 2;
  }
  (s->p[hole_index] = e)->min_heap_idx = hole_index;
}

void min_heap_shift_down(qnode_min_heap_t* s, unsigned hole_index, struct qnode_event_t* e) {
  unsigned min_child = 2 * (hole_index + 1);
  while(min_child <= s->n) {
    min_child -= min_child == s->n || min_heap_elem_greater(s->p[min_child], s->p[min_child - 1]);
    if(!(min_heap_elem_greater(e, s->p[min_child])))
      break;
    (s->p[hole_index] = s->p[min_child])->qnode_min_heap_idx = hole_index;
    hole_index = min_child;
    min_child = 2 * (hole_index + 1);
  }
  min_heap_shift_up(s, hole_index,  e);
}


int qnode_min_heap_push(qnode_min_heap_t* s, struct qnode_event_t* e) {
  if(qnode_min_heap_reserve(s, s->n + 1)) {
    return -1;
  }
  min_heap_shift_up(s, s->n++, e);
  return 0;
}

struct qnode_event_t* qnode_min_heap_pop(qnode_min_heap_t* s) {
  if(s->n) {
    struct qnode_event_t* e = *s->p;
    min_heap_shift_down(s, 0u, s->p[--s->n]);
    e->min_heap_idx = -1;
    return e;
  }
  return 0;
}

int qnode_min_heap_erase(qnode_min_heap_t* s, struct qnode_event_t* e) {
  if(((unsigned int)-1) != e->min_heap_idx) {
    struct qnode_event_t *last = s->p[--s->n];
    unsigned parent = (e->min_heap_idx - 1) / 2;
    if (e->min_heap_idx > 0 && min_heap_elem_greater(s->p[parent], last))
      min_heap_shift_up_(s, e->min_heap_idx, last);
    else
      min_heap_shift_down(s, e->min_heap_idx, last);
    e->min_heap_idx = -1;
    return 0;
  }
  return -1;
}
