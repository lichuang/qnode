/*
 * See Copyright Notice in qnode.h
 */

#include "qapi.h"

extern luaL_Reg buffer_apis[];
extern luaL_Reg node_apis[];
extern luaL_Reg net_apis[];

typedef luaL_Reg* api_array;

static api_array array[] = {
  &buffer_apis[0],
  &node_apis[0],
  &net_apis[0],
  NULL
};

void qapi_register(lua_State *state, struct qactor_t *actor) {
  int i, j;
  for (i = 0; array[i] != NULL; ++i) {
    for (j = 0; array[i][j].name != NULL; ++j) {
      const char *name = array[i][j].name;
      lua_CFunction func = array[i][j].func;
      lua_register(state, name, func);
    }
  }

  lua_pushlightuserdata(state, actor);
  lua_setglobal(state, "qnode");
}
