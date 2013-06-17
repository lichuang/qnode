/*
 * See Copyright Notice in qnode.h
 */

#include <lauxlib.h>
#include <stdlib.h>
#include <string.h>
#include "qactor.h"
#include "qluautil.h"

static int qstring_toul(lua_State *state) {
  unsigned long long result;
  const char        *input;
  char              *endptr;
  qactor_t          *actor;

  actor = qlua_get_actor(state);
  input = lua_tostring(state, 1);

  result = strtoull(input, &endptr, 10);

  lua_pushnumber(state, result);
  return 1;
}

luaL_Reg string_apis[] = {
  {"qstring_toul",  qstring_toul},
  {NULL, NULL},
};
