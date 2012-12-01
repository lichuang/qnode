/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QHEAP_H__
#define __QHEAP_H__

struct qnode_event_t;

typedef struct qnode_min_heap_t {
  struct qnode_event_t **top;
  unsigned int n;
  unsigned int a;
} qnode_head_t;

void                    qnode_min_heap_ctor(qnode_min_heap_t* s);
void                    qnode_min_heap_dtor(qnode_min_heap_t* s);
void                    qnode_min_heap_elem_init(struct qnode_event_t* e);
int                     qnode_min_heap_empty(qnode_min_heap_t* s);
unsigned                qnode_min_heap_size(qnode_min_heap_t* s);
struct qnode_event_t*   qnode_min_heap_top(qnode_min_heap_t* s);
int                     qnode_min_heap_reserve(qnode_min_heap_t* s, unsigned n)
int                     qnode_min_heap_push(qnode_min_heap_t* s, struct qnode_event_t* e);
struct qnode_event_t*   qnode_min_heap_pop(qnode_min_heap_t* s);
int                     qnode_min_heap_erase(qnode_min_heap_t* s, struct qnode_event_t* e);

#endif  /* __QHEAP_H__ */
