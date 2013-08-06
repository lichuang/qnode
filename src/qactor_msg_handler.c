/*
 * See Copyright Notice in qnode.h
 */

#include "qactor.h"
#include "qalloc.h"
#include "qamsg.h"
#include "qcore.h"
#include "qdict.h"
#include "qdefines.h"
#include "qlog.h"
#include "qluautil.h"
#include "qtype.h"

static int actor_send_handler(qamsg_header_t *msg, qactor_t *actor);

qamsg_func_t* actor_msg_handlers[] = {
  &actor_send_handler,
};

static int
actor_send_handler(qamsg_header_t *header, qactor_t *actor) {
  qactor_msg_t  *actor_msg;
  qamsg_send_t  *msg;
  lua_State     *state;

  msg = (qamsg_send_t*)header;
  actor_msg = &(msg->actor_msg);
  state = actor->state;

  /*
   * if the state yield waiting for msg, push the msg
   * into stack and resume
   */
  if (lua_status(state) == LUA_YIELD && actor->waiting_msg) {
    actor->waiting_msg = 0;
    lua_newtable(state);
    qlua_dump_dict(state, actor_msg->arg_dict);
    qdict_destroy(actor_msg->arg_dict);
    qlua_resume(state, 1);
  } else {
    /* else add the msg to the actor msg list */
    qlist_add_tail(&actor_msg->entry, &(actor->msg_list));
  }

  qfree(msg);
  return 0;
}
