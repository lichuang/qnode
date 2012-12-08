/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QLIST_H__
#define __QLIST_H__

typedef struct qnode_list_t {
  struct qnode_list_t  *prev;
  struct qnode_list_t  *next;
} qnode_list_t;


#define qnode_list_init(q) \
  (q)->prev = q;           \
  (q)->next = q

#define qnode_list_empty(h) \
  (h == (h)->prev)

#define qnode_list_insert_head(h, x)    \
  (x)->next = (h)->next;                \
  (x)->next->prev = x;                  \
  (x)->prev = h;                        \
  (h)->next = x

#define qnode_list_insert_after   qnode_list_insert_head

#define qnode_list_insert_tail(h, x)    \
  (x)->prev = (h)->prev;                \
  (x)->prev->next = x;                  \
  (x)->next = h;                        \
  (h)->prev = x

#define qnode_list_head(h)              \
  (h)->next

#define qnode_list_last(h)              \
  (h)->prev

#define qnode_list_sentinel(h)          \
  (h)

#define qnode_list_next(q)              \
  (q)->next

#define qnode_list_prev(q)              \
  (q)->prev

#define qnode_list_remove(x)            \
  (x)->next->prev = (x)->prev;          \
  (x)->prev->next = (x)->next;          \
  (x)->prev = NULL;                     \
  (x)->next = NULL

#define qnode_list_split(h, q, n)       \
  (n)->prev = (h)->prev;                \
  (n)->prev->next = n;                  \
  (n)->next = q;                        \
  (h)->prev = (q)->prev;                \
  (h)->prev->next = h;                  \
  (q)->prev = n;

#define qnode_list_add(h, n)            \
  (h)->prev->next = (n)->next;          \
  (n)->next->prev = (h)->prev;          \
  (h)->prev = (n)->prev;                \
  (h)->prev->next = h;

#define qnode_list_data(q, type, link)  \
  (type *) ((char *)q - offsetof(type, link))

#endif  /* __QLIST_H__ */
