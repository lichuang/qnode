/*
 * See Copyright Notice in qnode.h
 */

#include "qalloc.h"
#include "qamsg.h"
#include "qlog.h"
#include "qluautil.h"
#include "qtype.h"
#include "qwmsg.h"
#include "qworker.h"

static qamsg_header_t* qamsg_new(qid_t src, qid_t dst,
                                 int size, int type);

qmsg_t*
qamsg_send_new(lua_State *state, qid_t src, qid_t dst) {
  qactor_msg_t   *actor_msg;
  qmsg_t         *msg;
  qamsg_send_t   *send;
  qamsg_header_t *header;

  header = qamsg_new(src, dst, sizeof(qamsg_send_t), A_SEND);
  if (header == NULL) {
    return NULL;
  }
  send            = (qamsg_send_t*)header;
  actor_msg       = &(send->actor_msg);
  qlist_entry_init(&(actor_msg->entry));

  actor_msg->arg_dict = qdict_new(5);
  qlua_copy_table(state, 2, actor_msg->arg_dict);
  qdict_setnum(actor_msg->arg_dict, "src", src);

  msg = qwmsg_actor_new(decode_pid(src), decode_pid(dst), send);

  return msg;
}

static qamsg_header_t*
qamsg_new(qid_t src, qid_t dst, int size, int type) {
  qamsg_header_t *header;

  header = (qamsg_header_t*)qcalloc(size);
  if (header == NULL) {
    qerror("alloc type %d amsg error", type);
    return NULL;
  }

  header->src   = src;
  header->dst   = dst;
  header->type  = type;

  return header;
}

void
qactor_msg_destroy(qactor_msg_t *msg) {
  qdict_destroy(msg->arg_dict);
}
