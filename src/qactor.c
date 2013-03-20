/*
 * See Copyright Notice in qnode.h
 */

#include <stdint.h>
#include "qactor.h"
#include "qapi.h"
#include "qassert.h"
#include "qdefines.h"
#include "qdescriptor.h"
#include "qdict.h"
#include "qengine.h"
#include "qluautil.h"
#include "qlog.h"
#include "qidmap.h"
#include "qmailbox.h"
#include "qmempool.h"
#include "qmsg.h"
#include "qnet.h"
#include "qserver.h"
#include "qthread.h"

qid_t
qactor_new_id() {
  qmutex_lock(&g_server->id_map_mutex);
  qid_t id = qid_new(&g_server->id_map);
  qmutex_unlock(&g_server->id_map_mutex);
  return id;
}

qactor_t*
qactor_new(qmem_pool_t *pool, qid_t aid) {
  qactor_t *actor = qcalloc(pool, sizeof(qactor_t));
  if (actor == NULL) {
    return NULL;
  }
  actor->state = NULL;
  actor->pool  = pool;
  qlist_entry_init(&(actor->entry));
  qlist_entry_init(&(actor->desc_list));
  qlist_entry_init(&(actor->msg_list));
  actor->aid = aid;
  actor->parent = QID_INVALID;
  actor->waiting_netio = 0;
  actor->waiting_msg   = 0;
  qspinlock_init(&(actor->desc_list_lock));
  qserver_new_actor(actor);
  return actor;
}

void
qactor_destroy(qactor_t *actor) {
  qassert(actor);
  lua_close(actor->state);
  qmutex_lock(&g_server->id_map_mutex);
  qid_free(&(g_server->id_map), actor->aid);
  qmutex_unlock(&g_server->id_map_mutex);
  qspinlock_lock(&(actor->desc_list_lock));
  qlist_t *pos, *next;
  for (pos = actor->desc_list.next; pos != &(actor->desc_list); ) {
    qdescriptor_t *desc = qlist_entry(pos, qdescriptor_t, entry);
    next = pos->next;
    qdescriptor_destroy(desc);
    pos  = next;
  }
  qspinlock_unlock(&(actor->desc_list_lock));
  qspinlock_destroy(&(actor->desc_list_lock));
  for (pos = actor->msg_list.next; pos != &(actor->msg_list); ) {
    qmsg_t *msg = qlist_entry(pos, qmsg_t, entry);
    next = pos->next;
    qmsg_destroy(msg);
    pos  = next;
  }
  if (actor->listen_params != NULL) {
    qdict_destroy(actor->listen_params);
  }
  qfree(actor->pool, actor, sizeof(qactor_t));
}

void
qactor_attach(qactor_t *actor, lua_State *state) {
  qassert(actor->state == NULL);
  qapi_register(state, actor);
  actor->state = state;
}

qid_t
qactor_spawn(qactor_t *actor, lua_State *state) {
  qassert(actor);
  qtid_t receiver_id = qserver_worker_thread();
  qmsg_t *msg = qmsg_new(actor->tid, receiver_id);
  if (msg == NULL) {
    return QID_INVALID;
  }
  qid_t aid = qactor_new_id();
  if (aid == QID_INVALID) {
    free(msg);
    return -1;
  }
  qid_t parent = actor->aid;
  qmsg_init_spawn(msg, aid, parent, state);
  qactor_t *new_actor = qactor_new(actor->pool, aid);
  if (new_actor == NULL) {
    return -1;
  }
  qactor_attach(new_actor, state);
  new_actor->parent = actor->aid;
  msg->args.spawn.actor = new_actor;
  qmsg_send(msg);
  return aid;
}

qengine_t*
qactor_get_engine(qactor_t *actor) {
  qassert(actor);
  qassert(actor->tid > 0);
  return g_server->threads[actor->tid]->engine;
}

qthread_t*
qactor_get_thread(qactor_t *actor) {
  qassert(actor);
  return g_server->threads[actor->tid];
}
