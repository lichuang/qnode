/*
 * See Copyright Notice in qnode.h
 */

#include <lua.h>
#include <stdint.h>
#include "qalloc.h"
#include "qactor.h"
#include "qapi.h"
#include "qassert.h"
#include "qdefines.h"
#include "qengine.h"
#include "qluautil.h"
#include "qlog.h"
#include "qidmap.h"
#include "qmsg.h"
#include "qmmsg.h"
#include "qnet.h"
#include "qserver.h"
#include "qsocket.h"
#include "qwmsg.h"
#include "qworker.h"

qactor_t*
qactor_new(qid_t aid) {
  qactor_t  *actor;

  actor = qalloc(sizeof(qactor_t));
  if (actor == NULL) {
    return NULL;
  }
  actor->timers = qdict_new(10);
  if (actor->timers == NULL) {
    qfree(actor);
    return NULL;
  }

  actor->state = NULL;
  actor->listen_params = NULL;
  qlist_entry_init(&(actor->actor_entry));
  qlist_entry_init(&(actor->sock_list));
  qlist_entry_init(&(actor->msg_list));
  actor->aid = aid;
  actor->parent = QINVALID_ID;
  actor->waiting_netio = 0;
  actor->waiting_msg   = 0;
  actor->active        = 1;
  actor->ref           = -1;
  qspinlock_init(&(actor->sock_list_lock));
  qworker_add(aid, actor);

  return actor;
}

void
qactor_destroy(qactor_t *actor) {
  qlist_t   *pos, *next;
  qmsg_t    *msg;
  qsocket_t *socket;

  qspinlock_lock(&(actor->sock_list_lock));
  for (pos = actor->sock_list.next; pos != &(actor->sock_list); ) {
    socket = qlist_entry(pos, qsocket_t, entry);
    next = pos->next;
    qsocket_free(socket);
    pos  = next;
  }
  qspinlock_unlock(&(actor->sock_list_lock));
  qspinlock_destroy(&(actor->sock_list_lock));
  for (pos = actor->msg_list.next; pos != &(actor->msg_list); ) {
    msg = qlist_entry(pos, qmsg_t, entry);
    next = pos->next;
    qmsg_destroy(msg);
    pos  = next;
  }
  if (actor->listen_params != NULL) {
    qdict_free(actor->listen_params);
  }
  qworker_delete(actor);
  qdict_free(actor->timers);
  qfree(actor);
}

void
qactor_attach(qactor_t *actor, lua_State *state) {
  qassert(actor->state == NULL);
  lua_pushlightuserdata(state, state);
  lua_pushlightuserdata(state, actor);
  lua_settable(state, LUA_REGISTRYINDEX);
  actor->state = state;
}

qid_t
qactor_spawn(qactor_t *actor, lua_State *state, int ref) {
  qid_t            worker_id;
  qid_t            aid;
  qmsg_t          *msg;
  qactor_t        *new_actor;
  qworker_t       *worker;

  worker_id = qserver_worker();
  worker    = workers[worker_id];
  aid       = qworker_new_aid(worker);

  new_actor = qactor_new(aid);
  if (new_actor == NULL) {
    return QINVALID_ID;
  }
  new_actor->ref = ref;

  msg = qwmsg_spawn_new(new_actor, actor, state,
                        actor->tid, worker_id);
  if (msg == NULL) {
    qactor_destroy(new_actor);
    return QINVALID_ID;
  }
  qworker_send(msg);

  return aid;
}

qactor_t*
qactor_get(qid_t aid) {
  qworker_t *worker;

  worker = workers[decode_pid(aid)];
  return qworket_get_actor(worker, decode_id(aid));
}

qengine_t*
qactor_get_engine(qid_t aid) {
  qactor_t *actor;

  actor = qactor_get(aid);
  if (actor) {
    return workers[actor->tid]->engine;
  }

  return NULL;
}

qworker_t*
qactor_get_worker(qid_t aid) {
  qactor_t *actor;

  actor = qactor_get(aid);
  if (actor) {
    return workers[actor->tid];
  }

  return NULL;
}
