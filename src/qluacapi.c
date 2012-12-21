/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qactor.h"
#include "qluacapi.h"
#include "qluautil.h"
#include "qlog.h"
#include "qstring.h"

static qactor_t* get_actor(lua_State *state) {
  lua_getglobal(state, "c_actor");
  return (qactor_t*)lua_touserdata(state, -1);
}

/*
 * spawn an actor, return the actor ID
 * */
static int c_spawn(lua_State *state) {
  qactor_t *actor = get_actor(state);
  const char *mod = lua_tostring(state, 1);
  const char *fun = lua_tostring(state, 2);
  lua_State *new_state = qlua_new_state();
  actor->state = new_state;
  qstring_t string;
  qstring_init(&string);
  qstring_assign(&string, mod);
  qstring_append(&string, ".lua");
  if (qlua_dofile(actor, string.data) != 0) {
    qerror("load script to launce error");
  }
  qstring_destroy(&string);

  /* copy args table */
  lua_newtable(new_state);
  lua_pushvalue(new_state, 1);
  if (lua_istable(state, 3)) {
    lua_pushnil(state);
    size_t len;
    const char *key;
    const char *str_val = NULL;
    double num_val = 0;
    while (lua_next(state, 3)) {
      if (lua_isstring(state, -1)) {
        str_val = lua_tolstring(state, -1, &len);
      } else if (lua_isnumber(state, -1)) {
        num_val = lua_tonumber(state, -1);
      } else {
        qerror("child arg table val MUST be number or string");
        return -1;
      }

      key = lua_tolstring(state, -2, &len);

      lua_pushstring(new_state, key);
      if (str_val) {
        lua_pushstring(new_state, str_val);
      } else {
        lua_pushnumber(new_state, num_val);
      }
      lua_rawset(new_state, -3);
      lua_pop(state, 1);

      str_val = NULL;
    }
  }

  lua_getglobal(new_state, mod);
  lua_getfield(new_state, -1, fun);
  /* push the args table */
  lua_pushvalue(new_state, 1);
  return qactor_spawn(actor, new_state);
}

static struct {
  const char *name;
  lua_CFunction func;
} apis[] = {
  {"c_spawn", c_spawn},
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
