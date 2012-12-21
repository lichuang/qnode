/*
 * See Copyright Notice in qnode.h
 */

#include "qaidmap.h"
#include "qmutex.h"

/* page size = 2^12 = 4K */
#define PAGE_SHIFT    12
#define PAGE_SIZE    (1UL << PAGE_SHIFT)

#define BITS_PER_BYTE        8
#define BITS_PER_PAGE        (PAGE_SIZE * BITS_PER_BYTE)
#define BITS_PER_PAGE_MASK    (BITS_PER_PAGE - 1)

typedef struct aidmap_t {
  unsigned int nr_free;
  char page[AID_MAX_DEFAULT];
} aidmap_t;

static aidmap_t aidmap = { AID_MAX_DEFAULT, {'0'} };
static int last_aid = -1;
static qmutex_t aidmap_mutex;

static int test_and_set_bit(int offset, void *addr) {
  unsigned long mask = 1UL << (offset & (sizeof(unsigned long) * BITS_PER_BYTE - 1));    
  unsigned long *p = ((unsigned long*)addr) + (offset >> (sizeof(unsigned long) + 1));
  unsigned long old = *p;    

  *p = old | mask;    

  return (old & mask) != 0;
}

static void clear_bit(int offset, void *addr) {
  unsigned long mask = 1UL << (offset & (sizeof(unsigned long) * BITS_PER_BYTE - 1));    
  unsigned long *p = ((unsigned long*)addr) + (offset >> (sizeof(unsigned long) + 1));
  unsigned long old = *p;    

  *p = old & ~mask;    
}

static int find_next_zero_bit(void *addr, int size, int offset) {
  unsigned long *p;
  unsigned long mask;

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

static int alloc_aid() {
  int aid = last_aid + 1;
  int offset = aid & BITS_PER_PAGE_MASK;

  if (!aidmap.nr_free) {
    return QAID_INVALID;
  }

  offset = find_next_zero_bit(&aidmap.page, BITS_PER_PAGE, offset);
  if (BITS_PER_PAGE != offset && !test_and_set_bit(offset, &aidmap.page)) {
    --aidmap.nr_free;
    last_aid = offset;
    return offset;
  }

  return QAID_INVALID;
}

void qaid_free(qaid_t aid) {
  int offset = aid & BITS_PER_PAGE_MASK;
  aidmap.nr_free++;
  clear_bit(offset, &aidmap.page);
}

void qaidmap_init() {
  qmutex_init(&aidmap_mutex);
}

void qaidmap_destroy() {
  qmutex_destroy(&aidmap_mutex);
}

qaid_t qaid_new() {
  qmutex_lock(&aidmap_mutex);
  qaid_t aid = alloc_aid();
  qmutex_unlock(&aidmap_mutex);
  return aid;
}
