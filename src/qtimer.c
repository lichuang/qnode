/*
 * See Copyright Notice in qnode.h
 */

#include <time.h>
#include "qalloc.h"
#include "qdefines.h"
#include "qengine.h"
#include "qtimer.h"

static inline void set_timer_heap_index(void *data, int index);
static inline int  get_timer_heap_index(void *data);
static inline int  compare_timer(void *data1, void *data2); 
static        void update_now_time(qtimer_manager_t *mng);

static inline void
set_timer_heap_index(void *data, int index) {
  qtimer_t *timer;

  timer = (qtimer_t*)data;
  timer->heap_index = index;
}

static inline int
get_timer_heap_index(void *data) {
  qtimer_t *timer;

  timer = (qtimer_t*)data;

  return timer->heap_index;
}

static inline int
compare_timer(void *data1, void *data2) {
  qtimer_t *timer1, *timer2;

  timer1 = (qtimer_t*)data1;
  timer2 = (qtimer_t*)data2;

  return (timer1->timeout > timer2->timeout);
}

static void
update_now_time(qtimer_manager_t *mng) {
  time_t now;

  now         = time(NULL);
  mng->now    = now;
  mng->now_ms = now * 1000; 
}

void
qtimer_manager_init(qtimer_manager_t *mng, qengine_t *engine) {
  update_now_time(mng);
  mng->engine = engine;
  qidmap_init(&(mng->id_map));
  qlist_entry_init(&(mng->free_list));
  qminheap_init(&(mng->min_heap), compare_timer,
                set_timer_heap_index, get_timer_heap_index);
}

void
qtimer_manager_free(qtimer_manager_t *mng) {
  qminheap_destroy(&(mng->min_heap));
}

qid_t
qtimer_add(qtimer_manager_t *mng, uint32_t timeout,
           qtimer_func_t *func, uint32_t cycle, void *arg) {
  qtimer_t *timer;
  qlist_t  *pos;

  if (qlist_empty(&(mng->free_list))) {
    timer = qalloc(sizeof(qtimer_t));
    if (timer == NULL) {
      return -1;
    }
  } else {
    pos = mng->free_list.next;
    qlist_del_init(pos);
    timer = qlist_entry(pos, qtimer_t, entry);
  }
  timer->id       = qid_new(&(mng->id_map));
  timer->timeout  = timeout + mng->now_ms;
  timer->handler  = func;
  timer->cycle    = cycle;
  timer->arg      = arg;
  qid_attach(&(mng->id_map), timer->id, timer);
  qminheap_push(&(mng->min_heap), timer);

  return timer->id;
}

int
qtimer_next(qtimer_manager_t *mng) {
  qtimer_t *timer;

  timer = (qtimer_t*)qminheap_top(&(mng->min_heap));
  if (timer == NULL) {
    return -1;
  }
  if (timer->timeout < mng->now_ms) {
    return 0;
  }

  return (timer->timeout - mng->now_ms);
}

void
qtimer_process(qtimer_manager_t *mng) {
  uint64_t  now;
  qtimer_t *timer;

  if (qminheap_empty(&(mng->min_heap))) {
    return;
  }

  update_now_time(mng);

  now = mng->now_ms;
  timer = (qtimer_t*)qminheap_top(&(mng->min_heap));
  while (now >= timer->timeout) {
    (timer->handler)(timer->arg);
    qminheap_pop(&(mng->min_heap));
    if (timer->cycle > 0) {
      timer->timeout = now + timer->cycle;
      qminheap_push(&(mng->min_heap), timer);
    } else {
      qtimer_del(mng, timer->id);
    }
    timer = (qtimer_t*)qminheap_top(&(mng->min_heap));
  }
}

int
qtimer_del(qtimer_manager_t *mng, qid_t id) {
  qtimer_t *timer;

  timer = (qtimer_t*)mng->id_map.data[id];
  qminheap_erase(&(mng->min_heap), timer->heap_index);
  qid_free(&(mng->id_map), id);
  qlist_add_tail(&(timer->entry), &(mng->free_list));

  return 0;
}
