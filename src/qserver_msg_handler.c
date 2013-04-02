/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qassert.h"
#include "qactor.h"
#include "qdefines.h"
#include "qlist.h"
#include "qlog.h"
#include "qluautil.h"
#include "qmsg.h"
#include "qserver.h"
#include "qthread.h"

static int server_handle_wrong_msg(qserver_t *server, qmsg_t *msg) {
  UNUSED(server);
  UNUSED(msg);
  qerror("handle server type %d msg error", msg->type);
  return 0;
}

static int server_handle_spawn_msg(qserver_t *server, qmsg_t *msg) {
  qmsg_t     *new_msg;
  qid_t       aid;
  lua_State  *state;
  qactor_t   *actor;

  qinfo("handle spawn msg");

  new_msg = qmsg_clone(msg);
  aid = msg->args.spawn.aid;
  state = msg->args.spawn.state;
  actor = qactor_new(aid);

  qactor_attach(actor, state);
  actor->parent = msg->args.spawn.parent;
  new_msg->args.spawn.actor = actor;
  new_msg->sender_id = QSERVER_THREAD_TID;
  new_msg->receiver_id = qserver_worker_thread();
  qmsg_send(new_msg);
  server->actors[aid] = actor;

  return 0;
}

qserver_msg_handler g_server_msg_handlers[] = {
  &server_handle_wrong_msg,         /* wrong */
  &server_handle_wrong_msg,         /* s_start, wrong */
  &server_handle_spawn_msg,         /* spawn */
};
