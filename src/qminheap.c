/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qmalloc.h"
#include "qminheap.h"

void qminheap_init(qminheap_t *heap, cmp_func_t cmp, set_func_t set, get_func_t get) {
  int i = 0;
  heap->data = (void**)qmalloc(QID_MAX * sizeof(void*)); 
  for (i = 0; i < QID_MAX; ++i) {
    heap->data[i] = NULL;
  }
  heap->size = heap->num = 0;
  heap->cmp  = cmp;
  heap->set  = set;
  heap->get  = get;
}

static int minheap_reserve(qminheap_t* heap, unsigned int size) {
  if (heap->size >= size) {
    return 0;
  }
  size = heap->size + 128;
  void *data = qrealloc(heap->data, sizeof(void*) * size);
  if (data == NULL) {
    return -1;
  }
  heap->data = data;
  heap->size = size;
  return 0;
}

static int minheap_shift_up(qminheap_t* heap, unsigned hole_index, void* data) {
  unsigned parent = (hole_index - 1) / 2;
  while (hole_index && (heap->cmp)(heap->data[parent], data)) {
    heap->data[hole_index] = heap->data[parent];
    (heap->set)(heap->data[parent], hole_index);
    hole_index = parent;
    parent = (hole_index - 1) / 2;
  }
  heap->data[hole_index] = data;
  (heap->set)(data, hole_index);
  return hole_index;
}

static void minheap_shift_down(qminheap_t* heap, unsigned hole_index, void* data) {
  unsigned min_child = 2 * (hole_index + 1);
  while(min_child <= heap->num) {
    if (min_child == heap->num ||
        heap->cmp(heap->data[min_child], heap->data[min_child - 1])) {
      min_child -= 1;
    }
    if(!(heap->cmp(data, heap->data[min_child]))) {
      break;
    }
    heap->data[hole_index] = heap->data[min_child];
    (heap->set)(heap->data[min_child], hole_index);
    hole_index = min_child;
    min_child = 2 * (hole_index + 1);
  }
  minheap_shift_up(heap, hole_index, data);
}

static int minheap_push(qminheap_t *heap, void *data) {
  if (minheap_reserve(heap, heap->num + 1) == -1) {
    return -1;
  }
  heap->num += 1;
  return minheap_shift_up(heap, heap->num, data);
}

static int minheap_erase(qminheap_t* heap, void *data) {
  if(heap->get(data) != -1) {
    void *last = heap->data[--heap->size];
    int index = heap->get(data);
    unsigned parent = (index - 1) / 2;
    if (index > 0 && (heap->cmp)(heap->data[parent], last)) {
      minheap_shift_up(heap, index, last);
    } else {
      minheap_shift_down(heap, index, last);
    }
    heap->set(data, -1);
    return 0;
  }
  return -1;
}

int qminheap_erase(qminheap_t *heap, int index) {
  void *data = heap->data[index];
  if (data == NULL) {
    return -1;
  }
  heap->num -= 1;
  return minheap_erase(heap, data);
}

void* qminheap_pop(qminheap_t *heap) {
  if (heap->num > 0) {
    void *data = heap->data[0];
    heap->num -= 1;
    minheap_shift_down(heap, 0, heap->data[heap->num]);
    return data;
  }
  return NULL;
}

void* qminheap_top(qminheap_t *heap) {
  if (heap->num > 0) {
    void *data = heap->data[0];
    return data;
  }
  return NULL;
}

int qminheap_push(qminheap_t *heap, void *data) {
  /*
  if () {
    return -1;
  }
  */
  return minheap_push(heap, data);
}
