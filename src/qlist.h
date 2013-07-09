/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QLIST_H__
#define __QLIST_H__

typedef struct qlist_t qlist_t;

struct qlist_t {
  qlist_t *next;
  qlist_t *prev;
};

static inline void
qlist_entry_init(qlist_t *entry) {
  entry->next = entry;
  entry->prev = entry;
}

static inline void
__list_add(qlist_t *entry, qlist_t *prev, qlist_t *next) {
  next->prev  = entry;
  entry->next = next;
  entry->prev = prev;
  prev->next  = entry;
}

static inline void
qlist_add(qlist_t *entry, qlist_t *head) {
  __list_add(entry, head, head->next);
}

static inline void
qlist_add_tail(qlist_t *entry, qlist_t *head) {
  __list_add(entry, head->prev, head);
}

static inline void
__list_del(qlist_t *prev, qlist_t *next) {
  next->prev = prev;
  prev->next = next;
}

static inline void
qlist_del(qlist_t *entry) {
  __list_del(entry->prev, entry->next);
}

static inline void
qlist_del_init(qlist_t *entry) {
  __list_del(entry->prev, entry->next);
  qlist_entry_init(entry); 
}

static inline int
qlist_empty(qlist_t *head) {
  return (head->next == head);
}

static inline void
qlist_assign(qlist_t *dst, qlist_t* src) {
  *dst = *src;
  if(qlist_empty(src)) {
    qlist_entry_init(dst);
  } else {
    src->next->prev = dst;
    src->prev->next = dst;
  } 
}

static inline void
qlist_splice(qlist_t *list, qlist_t *head) {
  qlist_t *first = list->next;

  if (first != list) {
    qlist_t *last = list->prev;
    qlist_t *at = head->next;

    first->prev = head;
    head->next = first;

    last->next = at;
    at->prev = last;
  }
}

static inline void
qlist_splice_tail(qlist_t *list, qlist_t *head) {
  if(!qlist_empty(list)) {
    qlist_t *first = list->next;
    qlist_t *last = list->prev;
    qlist_t *at = head->prev;

    at->next = first;
    first->prev = at;

    last->next = head;
    head->prev = last;
  }
}

#define qlist_entry(ptr, type, member) \
  ((type *)((char *)(ptr) - ((unsigned long)(&((type*)1)->member) - 1)))

#define qlist_for_each(pos, head) \
  for (pos = (head)->next; pos != (head); pos = pos->next)

#endif  /* __QLIST_H__ */
