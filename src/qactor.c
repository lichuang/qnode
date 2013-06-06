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
#include "qmailbox.h"
#include "qmsg.h"
#include "qmmsg.h"
#include "qnet.h"
#include "qserver.h"
#include "qwmsg.h"
#include "qworker.h"

qid_t
qactor_new_id() {
  qid_t id;

  qmutex_lock(&g_server->id_map_mutex);
  id = qid_new(&g_server->id_map);
  qmutex_unlock(&g_server->id_map_mutex);
  return id;
}

qactor_t*
qactor_new(qid_t aid) {
  qactor_t *actor;

  actor = qalloc(sizeof(qactor_t));
  if (actor == NULL) {
    return NULL;
  }

  actor->state = NULL;
  qlist_entry_init(&(actor->entry));
  qlist_entry_init(&(actor->desc_list));
  qlist_entry_init(&(actor->msg_list));
  actor->aid = aid;
  actor->parent = QINVALID_ID;
  actor->waiting_netio = 0;
  actor->waiting_msg   = 0;
  qspinlock_init(&(actor->desc_list_lock));
  qserver_new_actor(actor);

  return actor;
}

void
qactor_destroy(qactor_t *actor) {
  qlist_t *pos, *next;
  qmsg_t  *msg;

  lua_close(actor->state);
  qmutex_lock(&g_server->id_map_mutex);
  qid_free(&(g_server->id_map), actor->aid);
  qmutex_unlock(&g_server->id_map_mutex);
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
  qid_t            receiver_id;
  qid_t            aid;
  qid_t            parent;
  qmsg_t          *msg;
  qactor_t        *new_actor;

  receiver_id = qserver_worker();
  aid = qactor_new_id();
  if (aid == QINVALID_ID) {
    return -1;
  }

  parent = actor->aid;
  new_actor = qactor_new(aid);
  if (new_actor == NULL) {
    return -1;
  }

  msg = qwmsg_spawn_new(new_actor, actor, state,
                        actor->tid, receiver_id);
  if (msg == NULL) {
    qactor_destroy(new_actor);
    return QINVALID_ID;
  }
  qworker_send(msg);

  return aid;
}

qengine_t*
qactor_get_engine(qactor_t *actor) {
  qassert(actor);
  qassert(actor->tid > 0);
  return g_server->workers[actor->tid]->engine;
}

qworker_t*
qactor_get_worker(qactor_t *actor) {
  qassert(actor);
  return g_server->workers[actor->tid];
}

void
qactor_send(qmsg_t *msg) {
  qactor_t   *actor;
  qmailbox_t *box;

  actor = g_server->actors[msg->recver];
  box   = &(actor->box);

  qmailbox_add(box, msg);
}
