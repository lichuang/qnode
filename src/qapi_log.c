/*
 * See Copyright Notice in qnode.h
 */

#include <lauxlib.h>
#include "qlog.h"

static int qllog(lua_State *state);
static int qlerror(lua_State *state);

luaL_Reg log_apis[] = {
  {"qllog",   qllog},
  {"qlerror", qlerror},
  {NULL, NULL},
};

static int
qllog(lua_State *state) {
  const char* str;
  
  str = lua_tostring(state, 1);
  qinfo("%s", str);

  return 0;
}

static int
qlerror(lua_State *state) {
  const char* str;
  
  str = lua_tostring(state, 1);
  qerror("%s", str);

  return 0;
}
