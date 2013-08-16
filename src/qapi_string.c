/*
 * See Copyright Notice in qnode.h
 */

#include <lauxlib.h>
#include <stdlib.h>
#include <string.h>
#include "qactor.h"
#include "qluautil.h"

static int qlstring_toul(lua_State *state);

luaL_Reg string_apis[] = {
  {"qlstring_toul",  qlstring_toul},
  {NULL, NULL},
};

static int
qlstring_toul(lua_State *state) {
  unsigned long long result;
  const char        *input;
  char              *endptr;

  input = lua_tostring(state, 1);

  result = strtoull(input, &endptr, 10);

  lua_pushnumber(state, result);
  return 1;
}
