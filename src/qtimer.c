/*
 * See Copyright Notice in qnode.h
 */

#include <time.h>
#include "qalloc.h"
#include "qdefines.h"
#include "qengine.h"
#include "qtimer.h"

static        void update_now_time(qtimer_manager_t *mng);

static void
update_now_time(qtimer_manager_t *mng) {
  struct timeval   tv;

  gettimeofday(&tv, NULL);
  mng->now    = tv.tv_sec;
  mng->now_ms = tv.tv_usec / 1000 + tv.tv_sec * 1000;
  //printf("sec: %u, usec: %u, ms: %f\n", (unsigned int)tv.tv_sec, (unsigned int)tv.tv_usec, mng->now_ms);
}

void
qtimer_manager_init(qtimer_manager_t *mng, qengine_t *engine) {
  qfreelist_conf_t conf = QFREELIST_CONF("timer free list",
                                         sizeof(qtimer_t),
                                         10,
                                         NULL, NULL, NULL);

  update_now_time(mng);
  mng->engine = engine;
  qidmap_init(&(mng->id_map));
  qfreelist_init(&mng->free_list, &conf);
  qrbtree_init(&(mng->rbtree), &(mng->sentinel), qrbtree_insert_timer_value);
}

void
qtimer_manager_free(qtimer_manager_t *mng) {
  qfreelist_destroy(&mng->free_list);
}

qid_t
qtimer_add(qengine_t *engine, uint32_t timeout,
           qtimer_pt *func, qtimer_destroy_pt *destroy,
           uint32_t cycle, void *data) {
  qtimer_t         *timer;
  qtimer_manager_t *mng;

  mng = &engine->timer_mng;
  timer = (qtimer_t*)qfreelist_new(&mng->free_list);

  if (timer == NULL) {
    return -1;
  }
  timer->id       = qid_new(&(mng->id_map));
  timer->timeout  = timeout + mng->now_ms;
  timer->handler  = func;
  timer->cycle    = cycle;
  timer->data     = data;
  timer->destroy  = destroy;
  timer->node.key = timer->timeout;
  qid_attach(&(mng->id_map), timer->id, timer);
  qrbtree_insert(&(mng->rbtree), &(timer->node));

  return timer->id;
}

int
qtimer_next(qtimer_manager_t *mng) {
  qtimer_t *timer;
  qrbtree_node_t  *node, *root, *sentinel;

  sentinel = mng->rbtree.sentinel;
  root = mng->rbtree.root;
  if (sentinel == root) {
    return -1;
  }

  node = qrbtree_min(root, sentinel);
  timer = (qtimer_t *) ((char *) node - offsetof(qtimer_t, node));
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
  qrbtree_node_t  *node, *root, *sentinel;

  update_now_time(mng);

  now = mng->now_ms;
  sentinel = mng->rbtree.sentinel;
  while (1) {
    root = mng->rbtree.root;
    if (sentinel == root) {
      return;
    }

    node = qrbtree_min(root, sentinel);
    timer = (qtimer_t *)((char *) node - offsetof(qtimer_t, node));
    if (timer->timeout > mng->now_ms) {
      return;
    }

    (timer->handler)(timer->data);
    if (timer->cycle > 0) {
      timer->timeout = now + timer->cycle;
    } else {
      qtimer_del(mng->engine, timer->id);
    }
  }
}

int
qtimer_del(qengine_t *engine, qid_t id) {
  qtimer_t         *timer;
  qtimer_manager_t *mng;

  mng = &engine->timer_mng;
  timer = (qtimer_t*)mng->id_map.data[id];
  if (timer == NULL) {
    return -1;
  }
  if (timer->destroy) {
    timer->destroy(timer->data);
  }
  qrbtree_delete(&(mng->rbtree), &(timer->node));
  qid_detach(&(mng->id_map), id);
  qfreelist_free(&mng->free_list, timer);

  return 0;
}
