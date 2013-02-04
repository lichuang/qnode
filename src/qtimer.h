/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QTIMER_H__
#define __QTIMER_H__

#include "qidmap.h"
#include "qlist.h"

typedef struct qtimer_t {
  qlist_t entry;
  uint32_t slot;
  uint32_t expire;
  uint32_t cycle;
  qid_t    id;
  void *arg;
  qtimer_func_t timeout;
} qtimer_t;

#define QMAX_TIMER_SLOTS 65535

typedef struct qtimer_wheel_t {
  qlist_t timers[QMAX_TIMER_SLOTS]; /* timer wheel */
  uint32_t now;                     /* now time in ms */
  uint32_t number;                  /* number of timer */
  uint32_t index;                   /* current slot index */
  uint32_t next;                    /* next time slot index */
  qidmap_t id_map;
  qlist_t  free_list;               /* free timer list */
} qtimer_wheel_t;

void  qtimer_wheel_init(qtimer_wheel_t *wheel);
qid_t qtimer_add(qtimer_wheel_t *wheel, int timeout_ms, qtimer_func_t *func, int cycle, void *arg);
qid_t qtimer_del(qtimer_wheel_t *wheel, qid_t id);
int   qtimer_next(qtimer_wheel_t *wheel);
void  qtimer_run(qtimer_wheel_t *wheel);

#endif  /* __QTIMER_H__ */
