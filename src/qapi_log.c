/*
 * See Copyright Notice in qnode.h
 */

#include <lauxlib.h>
#include "qlog.h"

static int qllog(lua_State *state);

luaL_Reg log_apis[] = {
  {"qllog",  qllog},
  {NULL, NULL},
};

static int
qllog(lua_State *state) {
  const char* str;
  
  str = lua_tostring(state, 1);
  qinfo("%s", str);

  return 0;
}
