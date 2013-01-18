/*
 * See Copyright Notice in qnode.h
 */

#include "qactor.h"
#include "qassert.h"
#include "qdefines.h"
#include "qluacapi.h"
#include "qluautil.h"
#include "qlog.h"
#include "qidmap.h"
#include "qmailbox.h"
#include "qmalloc.h"
#include "qmsg.h"
#include "qnet.h"
#include "qserver.h"
#include "qsocket.h"
#include "qthread.h"

qid_t qactor_new_id() {
  qmutex_lock(&g_server->id_map_mutex);
  qid_t id = qid_new(&g_server->id_map);
  qmutex_unlock(&g_server->id_map_mutex);
  return id;
}

qactor_t *qactor_new(qid_t aid) {
  qactor_t *actor = qalloc_type(qactor_t);
  int i;
  if (actor == NULL) {
    return NULL;
  }
  actor->state = NULL;
  qlist_entry_init(&(actor->entry));
  qlist_entry_init(&(actor->conn_list));
  actor->aid = aid;
  actor->parent = QID_INVALID;
  actor->listen_fd = 0;
  for (i = 0; i < QMAX_LUA_API_REF; ++i) {
    actor->lua_ref[i] = QINVALID_LUA_REF;
  }
  qserver_new_actor(actor);
  return actor;
}

void qactor_destroy(qactor_t *actor) {
  qassert(actor);
  lua_close(actor->state);
  qfree(actor);
}

void qactor_attach(qactor_t *actor, lua_State *state) {
  qassert(actor->state == NULL);
  qluac_register(state, actor);
  actor->state = state;
}

qid_t qactor_spawn(qactor_t *actor, lua_State *state) {
  qassert(actor);
  qtid_t receiver_id = qserver_worker_thread();
  qmsg_t *msg = qmsg_new(actor->tid, receiver_id);
  if (msg == NULL) {
    return QID_INVALID;
  }
  qid_t aid = qactor_new_id();
  if (aid == QID_INVALID) {
    qfree(msg);
    return -1;
  }
  qid_t parent = actor->aid;
  qmsg_init_spawn(msg, aid, parent, state);
  qactor_t *new_actor = qactor_new(aid);
  qactor_attach(new_actor, state);
  new_actor->parent = actor->aid;
  msg->args.spawn.actor = new_actor;
  qmsg_send(msg);
  return aid;
}

static int actor_get_lua_ref(qactor_t *actor, int ref) {
  lua_State *state = actor->state;
  qassert(ref >= 0 && ref < QMAX_LUA_API_REF);
  qassert(state);
  int idx = actor->lua_ref[ref];
  if (idx == QINVALID_LUA_REF) {
    qerror("actor %d get ref on %d error", actor->aid, ref);
    return -1;
  }
  lua_rawgeti(state, LUA_REGISTRYINDEX, idx);
  if(lua_type(state, -1) != LUA_TFUNCTION) {
    qerror("actor %d get ref callback on %d error", actor->aid, ref);
    return -1;
  }
  return 0;
}

void qactor_accept(int fd, int flags, void *data) {
  qinfo("actor accept ....");
  UNUSED(fd);
  UNUSED(flags);
  qactor_t *actor = (qactor_t*)data;
  qassert(actor->listen_fd == fd);
  int sock = -1;
  while ((sock = qnet_tcp_accept(actor->listen_fd)) != -1) {
    qsocket_t *socket = qsocket_get(sock);
    socket->aid = actor->aid;
    qlist_add_tail(&socket->entry, &actor->conn_list);
  }
  if (actor_get_lua_ref(actor, LISTENER) < 0) {
    return;
  }
  lua_call(actor->state, 0, 0);
}

struct qsocket_t* qactor_get_socket(qactor_t *actor) {
  if (qlist_empty(&actor->conn_list)) {
    return NULL;
  }
  qlist_t *pos = actor->conn_list.next;
  qlist_del_init(pos);
  qsocket_t *socket = qlist_entry(pos, qsocket_t, entry);
  return socket;
}
