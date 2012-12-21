/*
 * See Copyright Notice in qnode.h
 */

#include "qactor.h"
#include "qaidmap.h"
#include "qassert.h"
#include "qluacapi.h"
#include "qluautil.h"
#include "qmailbox.h"
#include "qmalloc.h"
#include "qmsg.h"
#include "qserver.h"
#include "qthread.h"

qactor_t *qactor_new(qaid_t aid) {
  qassert(server);
  qactor_t *actor = qalloc_type(qactor_t);
  if (actor == NULL) {
    return NULL;
  }
  lua_State *state = qlua_new_state();
  qluac_register(state, actor);
  actor->state = state;
  qlist_entry_init(&(actor->entry));
  actor->aid = aid;
  actor->parent = QAID_INVALID;
  return actor;
}

void qactor_destroy(qactor_t *actor) {
  qassert(actor);
  lua_close(actor->state);
  qfree(actor);
}

qaid_t qactor_spawn(qactor_t *actor, lua_State *state) {
  qassert(actor);
  qassert(actor->thread);
  qmsg_t *msg = qmsg_new();
  if (msg == NULL) {
    return QAID_INVALID;
  }
  qaid_t aid = qaid_new();
  if (aid == QAID_INVALID) {
    qfree(msg);
    return aid;
  }
  qaid_t parent = actor->aid;
  qmsg_init_spawn(msg, aid, parent, state);
  qserver_add_mail(actor->thread->tid, msg);
  return aid;
}
