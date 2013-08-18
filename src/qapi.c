/*
 * See Copyright Notice in qnode.h
 */

#include "qapi.h"
#include "qluautil.h"
#include "qworker.h"

extern luaL_Reg buffer_apis[];
extern luaL_Reg debug_apis[];
extern luaL_Reg log_apis[];
extern luaL_Reg net_apis[];
extern luaL_Reg node_apis[];
extern luaL_Reg string_apis[];

typedef luaL_Reg* api_array;

static api_array array[] = {
  &buffer_apis[0],
  &debug_apis[0],
  &log_apis[0],
  &net_apis[0],
  &node_apis[0],
  &string_apis[0],
  NULL
};

void
qapi_register(lua_State *state) {
  int i, j;
  const char *name;

  for (i = 0; array[i] != NULL; ++i) {
    for (j = 0; array[i][j].name != NULL; ++j) {
      name = array[i][j].name;
      lua_CFunction func = array[i][j].func;
      lua_register(state, name, func);
    }
  }
}
