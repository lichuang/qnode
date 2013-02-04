/*
 * See Copyright Notice in qnode.h
 */

#include "qdefines.h"
#include "qtimer.h"
#include "qmalloc.h"

void qtimer_wheel_init(qtimer_wheel_t *wheel) {
  int i;
  wheel->number = 0;
  wheel->index  = 0;
  wheel->next   = 0;
  for (i = 0; i < QMAX_TIMER_SLOTS; ++i) {
    qlist_entry_init(&(wheel->timers[i]));
  }
  qlist_entry_init(&(wheel->free_list));
  qidmap_init(&wheel->id_map);
}

static qtimer_t* timer_new(qtimer_wheel_t *wheel, uint32_t expire,
                           qtimer_func_t *func, int cycle, void *arg) {
  qtimer_t *timer = NULL;
  if (qlist_empty(&(wheel->free_list))) {
    timer = qalloc_type(qtimer_t);
  } else {
    qlist_t* pos = wheel->free_list;
    qlist_del_init(pos);
    timer = list_entry(pos, qtimer_t, entry);
  }
  timer->timeout  = func;
  timer->expire   = expire;
  timer->slot     = 0;
  timer->cycle    = cycle;
  timer->arg      = arg;
  timer->id       = qid_new(&wheel->id_map);
  qlist_entry_init(&(timer->entry));
  return timer;
}

static void internal_add_timer(qtimer_wheel_t *wheel, qtimer_t *timer) {
  uint32_t slot   = (wheel->index + timeout_ms) % QMAX_TIMER_SLOTS;
  timer->slot     = slot;
  qlist_add(&(timer->entry), &(wheel->timers[slot]));
  ++(wheel->number);
}

qid_t qtimer_add(qtimer_wheel_t *wheel, int timeout_ms,
                 qtimer_func_t *func, int cycle, void *arg) {
  uint32_t expire = timeout_ms + wheel->now;
  qtimer_t *timer = timer_new(wheel, expire, func, cycle, arg);
  internal_add_timer(wheel, timer);
  return timer->id;
}

qid_t qtimer_del(qtimer_wheel_t *wheel, qid_t id) {
}

int qtimer_next(qtimer_wheel_t *wheel) {
  /* no timer ? */
  if (wheel->number == 0) {
    wheel->next = 0;
    return -1;
  }
  int next = wheel->index;
  while(1) {
    qlist_t *list = wheel->timers[next];
    if (!qlist_empty(list)) {
      qlist_t *pos = NULL;
      qtimer_t *timer;
      int min = 0;
      qlist_for_each(pos, list) {
        timer = qlist_entry(pos, qtimer_t, entry);
        int diff = timer->expire - wheel->now;
        if (diff == next) {
          min = 0;
          break;
        }
        min = min > diff ? diff : min;
      }
      if (min != 0) {
        next = min;
      }
      break;
    }
    int next_slot = (next + 1) % QMAX_TIMER_SLOTS;
    if (next_slot == wheel->index) {
      break;
    }
    next = next_slot;
  }

  wheel->next = next;
  return count;
}

static void run_timer_list(qtimer_wheel_t *wheel, qlist_t *list) {
  qlist_t *pos = NULL, *next = NULL;
  qtimer_t *timer;
  for (pos = list->next; pos != list; ) {
    timer = qlist_entry(pos, qtimer_t, entry);
    next  = pos->next;
    int diff = timer->expire - wheel->now;
    if (diff > 0) {
      continue;
    }
    (timer->timeout)(timer->arg);
    if (timer->cycle == 0) {
      qlist_del_init(pos);
      qlist_add(pos, wheel->free_list);
    }
    pos = next;
  }
}

void  qtimer_run(qtimer_wheel_t *wheel) {
  if (wheel->next == wheel->index) {
    return;
  }
  int index = wheel->index;
  while (1) {
    qlist_t *list = wheel->timers[index];
    if (!qlist_empty(list)) {
      run_timer_list(wheel, list);
    }
    index = (index + 1) % QMAX_TIMER_SLOTS;
    if (index == wheel->next) {
      break;
    }
  }
  wheel->index = wheel->next;
}
