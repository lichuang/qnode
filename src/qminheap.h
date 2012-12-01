/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QMINHEAP_H__
#define __QMINHEAP_H__

struct qnode_event_t;

typedef struct qnode_minheap_t {
  struct qnode_event_t **top;
  unsigned int n;
  unsigned int a;
} qnode_minheap_t;

void                    qnode_minheap_ctor(qnode_minheap_t* s);
void                    qnode_minheap_dtor(qnode_minheap_t* s);
void                    qnode_minheap_elem_init(struct qnode_event_t* e);
int                     qnode_minheap_empty(qnode_minheap_t* s);
unsigned int            qnode_minheap_size(qnode_minheap_t* s);
struct qnode_event_t*   qnode_minheap_top(qnode_minheap_t* s);
int                     qnode_minheap_reserve(qnode_minheap_t* s, unsigned int n);
int                     qnode_minheap_push(qnode_minheap_t* s, struct qnode_event_t* e);
struct qnode_event_t*   qnode_minheap_pop(qnode_minheap_t* s);
int                     qnode_minheap_erase(qnode_minheap_t* s, struct qnode_event_t* e);

#endif  /* __QMINHEAP_H__ */
