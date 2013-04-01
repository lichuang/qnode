/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qassert.h"
#include "qidmap.h"

/* page size = 2^12 = 4K */
#define PAGE_SHIFT    12
#define PAGE_SIZE    (1UL << PAGE_SHIFT)

#define BITS_PER_BYTE        8
#define BITS_PER_PAGE        (PAGE_SIZE * BITS_PER_BYTE)
#define BITS_PER_PAGE_MASK    (BITS_PER_PAGE - 1)

static int test_and_set_bit(int offset, void *addr) {
  unsigned long   mask, old;
  unsigned long  *p;

  mask = 1UL << (offset & (sizeof(unsigned long) * BITS_PER_BYTE - 1));    
  p = ((unsigned long*)addr) + (offset >> (sizeof(unsigned long) + 1));
  old = *p;    
  *p = old | mask;    

  return (old & mask) != 0;
}

static void clear_bit(int offset, void *addr) {
  unsigned long   mask, old;
  unsigned long  *p;

  mask = 1UL << (offset & (sizeof(unsigned long) * BITS_PER_BYTE - 1));    
  p = ((unsigned long*)addr) + (offset >> (sizeof(unsigned long) + 1));
  old = *p;    

  *p = old & ~mask;    
}

static int find_next_zero_bit(void *addr, int size, int offset) {
  unsigned long  *p;
  unsigned long   mask;

  while (offset < size) {
    p = ((unsigned long*)addr) + (offset >> (sizeof(unsigned long) + 1));
    mask = 1UL << (offset & (sizeof(unsigned long) * BITS_PER_BYTE - 1));    

    if ((~(*p) & mask)) {
      break;
    }

    ++offset;
  }

  return offset;
}

static int alloc_qid(qidmap_t *idmap) {
  int qid;
  int offset;

  qid = idmap->last_qid + 1;
  offset = qid & BITS_PER_PAGE_MASK;
  if (!idmap->nr_free) {
    return QID_INVALID;
  }

  offset = find_next_zero_bit(&idmap->page, BITS_PER_PAGE, offset);
  if (BITS_PER_PAGE != offset && !test_and_set_bit(offset, &idmap->page)) {
    --idmap->nr_free;
    idmap->last_qid = offset;
    return offset;
  }

  return QID_INVALID;
}

void qid_free(qidmap_t *idmap, qid_t id) {
  int offset;

  offset = id & BITS_PER_PAGE_MASK;
  idmap->nr_free++;
  clear_bit(offset, &idmap->page);
  idmap->data[id] = NULL;
}

void qidmap_init(qidmap_t *idmap) {
  int i;

  idmap->nr_free = QID_MAX;
  for (i = 0; i < QID_MAX; ++i) {
    idmap->page[i] = '0';
    idmap->data[i] = NULL;
  }
  idmap->last_qid = QID_INVALID;
}

qid_t qid_new(qidmap_t *idmap) {
  return alloc_qid(idmap);
}

void   qid_attach(qidmap_t *idmap, qid_t id, void *data) {
  qassert(idmap->data[id] == NULL);
  idmap->data[id] = data;
}
