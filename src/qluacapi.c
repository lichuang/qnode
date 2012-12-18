/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qactor.h"
#include "qluacapi.h"
#include "qluautil.h"

static qactor_t* get_actor(lua_State *state) {
  lua_getglobal(state, "c_actor");
  return (qactor_t*)lua_touserdata(state, -1);
}

/*
 * launch an actor, return the actor ID
 * */
static int c_launch(lua_State *state) {
  printf("c_launch\n");
  const char *mod = lua_tostring(state, 1);
  const char *fun = lua_tostring(state, 2);
  printf("mod name: %s, func: %s\n", mod, fun);
  qactor_t *actor = get_actor(state);
  qlua_init_path(actor);
  //qthread_t *thread = actor->thread;
  lua_State *new_state = qlua_new_state();
  if (luaL_dofile(new_state, "child") != 0) {
  }

  lua_getglobal(new_state, mod);
  //lua_getfield(new_state, -1, mod);
  lua_getfield(new_state, -1, fun);
  lua_call(new_state, 0, 0);

  return 0;
}

static struct {
  const char *name;
  lua_CFunction func;
} apis[] = {
  {"c_launch", c_launch},
  {NULL, NULL},
};

void qluac_register(lua_State *state, struct qactor_t *actor) {
  int i;
  for (i = 0; apis[i].name != NULL; ++i) {
    const char *name = apis[i].name;
    lua_CFunction func = apis[i].func;
    lua_register(state, name, func);
  }
  lua_pushlightuserdata(state, actor);
  lua_setglobal(state, "c_actor");
}
