/*
 * See Copyright Notice in qnode.h
 */

#include "qalloc.h"
#include "qamsg.h"
#include "qluautil.h"

qmsg_t* qamsg_send_new(lua_State *state, qid_t sender, qid_t recver) {
  qactor_msg_t *actor_msg;
  qmsg_t       *msg;
  qamsg_send_t *send;

  msg = qmsg_new(sender, recver, sizeof(qamsg_send_t), A_SEND);
  if (msg == NULL) {
    return NULL;
  }
  send            = (qamsg_send_t*)msg;
  actor_msg       = &(send->actor_msg);
  qlist_entry_init(&(actor_msg->entry));

  actor_msg->arg_dict = qdict_new(5);
  qlua_copy_table(state, 2, actor_msg->arg_dict);

  return msg;
}

void
qactor_msg_destroy(qactor_msg_t *msg) {
  qdict_destroy(msg->arg_dict);
}
