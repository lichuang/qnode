/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QLIST_H__
#define __QLIST_H__

typedef struct qnode_list_t {
  struct qnode_list_t *next;
  struct qnode_list_t *prev;
} qnode_list_t;

static inline void qnode_list_entry_init(qnode_list_t *entry) {
  entry->next = entry;
  entry->prev = entry;
}

static inline void __list_add(qnode_list_t *entry, qnode_list_t *prev, qnode_list_t *next) {
  next->prev  = entry;
  entry->next = next;
  entry->prev = prev;
  prev->next  = entry;
}

static inline void qnode_list_add(qnode_list_t *entry, qnode_list_t *head) {
  __list_add(entry, head, head->next);
}

static inline void qnode_list_add_tail(qnode_list_t *entry, qnode_list_t *head) {
  __list_add(entry, head->prev, head);
}

static inline void __list_del(qnode_list_t *prev, qnode_list_t *next) {
  next->prev = prev;
  prev->next = next;
}

static inline void qnode_list_del(struct qnode_list_t *entry) {
  __list_del(entry->prev, entry->next);
}

static inline void qnode_list_del_init(struct qnode_list_t *entry) {
  __list_del(entry->prev, entry->next);
  qnode_list_entry_init(entry); 
}

static inline int qnode_list_empty(struct qnode_list_t *head) {
  return (head->next == head);
}

static inline void qnode_list_assign(qnode_list_t *dest, qnode_list_t* src) {
  *dest = *src;
  if(qnode_list_empty(src)) {
    qnode_list_entry_init(dest);
  } else {
    src->next->prev = dest;
    src->prev->next = dest;
  } 
}

static inline void qnode_list_splice(qnode_list_t *list, qnode_list_t *head) {
  qnode_list_t *first = list->next;

  if (first != list) {
    qnode_list_t *last = list->prev;
    qnode_list_t *at = head->next;

    first->prev = head;
    head->next = first;

    last->next = at;
    at->prev = last;
  }
}

static inline void qnode_list_splice_tail(qnode_list_t *list, qnode_list_t *head) {
  if(!qnode_list_empty(list)) {
    qnode_list_t *first = list->next;
    qnode_list_t *last = list->prev;
    qnode_list_t *at = head->prev;

    at->next = first;
    first->prev = at;

    last->next = head;
    head->prev = last;
  }
}

#define qnode_list_entry(ptr, type, member) \
  ((type *)((char *)(ptr) - ((unsigned long)(&((type*)1)->member) - 1)))

#define qnode_list_for_each(pos, head) \
  for (pos = (head)->next; pos != (head); pos = pos->next)

#endif  /* __QLIST_H__ */
