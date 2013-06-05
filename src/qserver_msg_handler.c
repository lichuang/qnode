/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qassert.h"
#include "qactor.h"
#include "qcore.h"
#include "qdefines.h"
#include "qlist.h"
#include "qlog.h"
#include "qluautil.h"
#include "qmsg.h"
#include "qserver.h"
#include "qworker.h"

static int
server_handle_wrong_msg(qmsg_t *msg, void *reader) {
  UNUSED(reader);
  UNUSED(msg);
  qerror("handle server type %d msg error", msg->type);
  return 0;
}

static int
server_handle_spawn_msg(qmsg_t *msg, void *reader) {
  qserver_t  *server;
  qmsg_t     *new_msg;
  qid_t       aid;
  lua_State  *state;
  qactor_t   *actor;

  qinfo("handle spawn msg");

  server  = (qserver_t*)reader;
  new_msg = qmsg_clone(msg);
  aid = msg->args.spawn.aid;
  state = msg->args.spawn.state;
  actor = qactor_new(aid);

  qactor_attach(actor, state);
  actor->parent = msg->args.spawn.parent;
  new_msg->args.spawn.actor = actor;
  new_msg->sender_id = QMAIN_THREAD_TID;
  new_msg->receiver_id = qserver_worker();
  qworker_send(new_msg->receiver_id, new_msg);
  server->actors[aid] = actor;

  return 0;
}

static int
server_handle_signal_msg(qmsg_t *msg, void *reader) {
  int signo;
  
  UNUSED(reader);
  signo = msg->args.signal.signo;

  return 0;
}

qmsg_func_t* g_server_msg_handlers[] = {
  &server_handle_wrong_msg,   /* wrong */
  &server_handle_wrong_msg,   /* start, wrong */
  &server_handle_spawn_msg,   /* spawn */
  &server_handle_wrong_msg,   /* send, wrong */
  &server_handle_signal_msg,  /* signal */
  &server_handle_wrong_msg,   /* log, wrong */
};
