/*
 * See Copyright Notice in qnode.h
 */

#include <stdint.h>
#include "qalloc.h"
#include "qactor.h"
#include "qapi.h"
#include "qassert.h"
#include "qdefines.h"
#include "qdescriptor.h"
#include "qengine.h"
#include "qluautil.h"
#include "qlog.h"
#include "qidmap.h"
#include "qmsg.h"
#include "qmmsg.h"
#include "qnet.h"
#include "qserver.h"
#include "qwmsg.h"
#include "qworker.h"

qactor_t*
qactor_new(qid_t aid) {
  qactor_t  *actor;

  actor = qalloc(sizeof(qactor_t));
  if (actor == NULL) {
    return NULL;
  }

  actor->state = NULL;
  actor->listen_params = NULL;
  qlist_entry_init(&(actor->entry));
  qlist_entry_init(&(actor->desc_list));
  qlist_entry_init(&(actor->msg_list));
  actor->aid = aid;
  actor->parent = QINVALID_ID;
  actor->waiting_netio = 0;
  actor->waiting_msg   = 0;
  qspinlock_init(&(actor->desc_list_lock));
  qworker_add(aid, actor);

  return actor;
}

void
qactor_destroy(qactor_t *actor) {
  qlist_t *pos, *next;
  qmsg_t  *msg;

  //lua_close(actor->state);
  qspinlock_lock(&(actor->desc_list_lock));
  for (pos = actor->desc_list.next; pos != &(actor->desc_list); ) {
    qdescriptor_t *desc = qlist_entry(pos, qdescriptor_t, entry);
    next = pos->next;
    qdescriptor_destroy(desc);
    pos  = next;
  }
  qspinlock_unlock(&(actor->desc_list_lock));
  qspinlock_destroy(&(actor->desc_list_lock));
  for (pos = actor->msg_list.next; pos != &(actor->msg_list); ) {
    msg = qlist_entry(pos, qmsg_t, entry);
    next = pos->next;
    qmsg_destroy(msg);
    pos  = next;
  }
  if (actor->listen_params != NULL) {
    qdict_destroy(actor->listen_params);
  }
  qworker_delete(actor->aid);
  qfree(actor);
}

void
qactor_attach(qactor_t *actor, lua_State *state) {
  qassert(actor->state == NULL);
  qapi_register(state, actor);
  actor->state = state;
}

qid_t
qactor_spawn(qactor_t *actor, lua_State *state) {
  qid_t            worker_id;
  qid_t            aid;
  qid_t            parent;
  qmsg_t          *msg;
  qactor_t        *new_actor;
  qworker_t       *worker;

  worker_id = qserver_worker();
  worker    = server->workers[worker_id];
  aid       = qworker_new_aid(worker);

  parent = actor->aid;
  new_actor = qactor_new(aid);
  if (new_actor == NULL) {
    return -1;
  }

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

  worker = server->workers[decode_pid(aid)];
  return qworket_get_actor(worker, decode_id(aid));
}

qengine_t*
qactor_get_engine(qid_t aid) {
  qactor_t *actor;

  actor = qactor_get(aid);
  if (actor) {
    return server->workers[actor->tid]->engine;
  }

  return NULL;
}

qworker_t*
qactor_get_worker(qid_t aid) {
  qactor_t *actor;

  actor = qactor_get(aid);
  if (actor) {
    return server->workers[actor->tid];
  }

  return NULL;
}
