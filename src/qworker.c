/*
 * See Copyright Notice in qnode.h
 */

#include <lua.h>
#include <lstate.h>
#include <lgc.h>
#include <unistd.h>
#include <stdio.h>
#include "qalloc.h"
#include "qassert.h"
#include "qactor.h"
#include "qconfig.h"
#include "qengine.h"
#include "qdefines.h"
#include "qlist.h"
#include "qlog.h"
#include "qluautil.h"
#include "qmutex.h"
#include "qserver.h"
#include "qworker.h"

extern qmsg_pt* worker_msg_handlers[];
qworker_t*      workers[QMAX_WORKER] = {NULL};

static int   worker_msg_handler(qmsg_t *msg, void *reader);
static void* worker_main(void *arg);
static void* worker_alloc(void *ud, void *ptr,
                          size_t osize, size_t nsize);
static void  recycle(void *data);
static void  free_actors(qworker_t *worker);
#ifdef DEBUG
static void  reload(lua_State *state, const char *file);
#endif

static int
worker_msg_handler(qmsg_t *msg, void *reader) {
  qworker_t *worker;

  worker = (qworker_t*)reader;
  //qinfo("worker %d handle %d msg", worker->tid, msg->type);
  return (*worker_msg_handlers[msg->type])(msg, reader);
}

static void*
worker_main(void *arg) {
  qworker_t *worker;

  worker = (qworker_t*)arg;
  worker->running = 1;
  (*worker->done)();
  qengine_loop(worker->engine);

  qmailbox_free(&(worker->box));
  free_actors(worker);
  qengine_destroy(worker->engine);
#ifdef DEBUG
  ldb_free(worker->ldb);
#endif
  lua_close(worker->state);

  return NULL;
}

qworker_t*
qworker_new(qid_t tid, qthread_start_pt done) {
  qworker_t *worker;

  worker = qcalloc(sizeof(qworker_t));
  if (worker == NULL) {
    printf("create worker error\n");
    return NULL;
  }
  worker->engine = qengine_new();
  if (worker->engine == NULL) {
    printf("create worker engine error\n");
    return NULL;
  }
  worker->actors = qcalloc(sizeof(qactor_t *) * MAX_ID);
  if (worker->actors == NULL) {
    printf("create worker actors error\n");
    return NULL;
  }
  qtimer_add(worker->engine, config.recycle_internal * 1000,
             recycle, NULL,
             config.recycle_internal * 1000, worker);
  qmailbox_init(&(worker->box), worker_msg_handler,
                worker->engine, worker);
  worker->tid = tid;
  worker->current = 0;
  worker->alloc = 0;
  qmutex_init(&(worker->mutex));
  /* create the lua VM for the worker */
  worker->state = qlua_new_state(worker_alloc, worker);
  if (worker->state == NULL) {
    return NULL;
  }

  /*
   * register a table to anchor lua coroutines reliably:
   * {([int]ref) = [cort]}
   */
  lua_pushlightuserdata(worker->state, &worker->coroutines_key);
  lua_newtable(worker->state);
  lua_rawset(worker->state, LUA_REGISTRYINDEX);

  worker->running = 0;
  worker->done = done;
  qlist_entry_init(&(worker->actor_list));
  pthread_create(&worker->id, NULL,
                 worker_main, worker);

#ifdef DEBUG
  worker->ldb = ldb_new(worker->state, reload);
#endif
  /*
  while (!worker->running) {
    usleep(100);
  }
  */
  return worker;
}

void
qworker_destroy(qworker_t *worker) {
  /* wait for the thread stop */
  pthread_join(worker->id, NULL);
  qmutex_destroy(&(worker->mutex));
  qfree(worker);
}

qid_t
qworker_new_aid(qworker_t *worker) {
  qid_t aid;
  qid_t current;

  qmutex_lock(&(worker->mutex));
  current = worker->current;
  while (worker->actors[current]) {
    ++current;
  }
  aid = encode_aid(current, worker->tid);
  qassert(decode_pid(aid) == worker->tid);
  qassert(decode_id(aid)  == current);
  worker->current = ++current % MAX_ID;
  qmutex_unlock(&(worker->mutex));

  return aid;
}

qactor_t*
qworket_get_actor(qworker_t *worker, qid_t id) {
  qactor_t *actor;

  qmutex_lock(&(worker->mutex));
  actor = worker->actors[id];
  qmutex_unlock(&(worker->mutex));

  return actor;
}

void
qworker_add(qid_t aid, qactor_t *actor) {
  qworker_t *worker;
  qid_t id;

  worker = workers[decode_pid(aid)];
  id = decode_id(aid);
  qmutex_lock(&(worker->mutex));
  worker->actors[id] = actor;
  qlist_add_tail(&(actor->actor_entry), &(worker->actor_list));
  qmutex_unlock(&(worker->mutex));
}

void
qworker_delete(qactor_t *actor) {
  qworker_t *worker;
  qid_t id, aid;

  aid = actor->aid;
  worker = workers[decode_pid(aid)];
  id = decode_id(aid);
  qmutex_lock(&(worker->mutex));
  //worker->actors[id]->active = 0;
  qlist_del(&(actor->actor_entry));
  worker->actors[id] = NULL;
  qmutex_unlock(&(worker->mutex));
}

void
qworker_send(qmsg_t *msg) {
  qworker_t  *worker;
  qmailbox_t *box;

  worker = workers[msg->recver];
  box    = &(worker->box);
  qmailbox_add(box, msg);
}

static void *
worker_alloc(void *ud, void *ptr, size_t osize, size_t nsize) {
  qworker_t *worker;

  worker = (qworker_t*)ud;
  worker->alloc += nsize - osize;
  //qstdout("worker %d, alloc: %d\n", worker->tid, worker->alloc);
  if (nsize == 0) {
    qfree(ptr);
    return NULL;
  } else {
    return qrealloc(ptr, nsize);
  }
}

static void
recycle(void *data) {
  qworker_t *worker;

  worker = (qworker_t*)data;
  //luaC_checkGC(worker->state);
  int oldsize = worker->alloc;
  luaC_fullgc(worker->state);
  qstdout("after recycle: worker %d old alloc: %d, alloc %d\n", worker->tid, oldsize, worker->alloc);
}

static void
free_actors(qworker_t *worker) {
  int       i;
  qactor_t *actor;
  
  for (i = 0; i < MAX_ID; ++i) {
    actor = worker->actors[i];
    if (!actor) {
      continue;
    }
    qactor_free(actor);
  }
  qfree(worker->actors);
  qmutex_destroy(&(worker->mutex));
}

#ifdef DEBUG
static void
reload(lua_State *state, const char *file) {
  qlua_reload(state, file);
}
#endif
