/*
 * See Copyright Notice in qnode.h
 */

#include "qactor.h"
#include "qassert.h"
#include "qluacapi.h"
#include "qluautil.h"
#include "qmalloc.h"
#include "qserver.h"

qactor_t *qactor_new(qserver_t *server) {
  qassert(server);
  qactor_t *actor = qalloc_type(qactor_t);
  if (actor == NULL) {
    return NULL;
  }
  lua_State *state = qlua_new_state();
  qluac_register(state, actor);
  //qlua_set_path(state, server->config->script_path);
  actor->state = state;
  qlist_entry_init(&(actor->entry));
  return actor;
}

void qactor_destroy(qactor_t *actor) {
  qassert(actor);
  lua_close(actor->state);
  qfree(actor);
}
