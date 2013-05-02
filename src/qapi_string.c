/*
 * See Copyright Notice in qnode.h
 */

#include <lauxlib.h>
#include <stdlib.h>
#include <string.h>

static int qnode_strtoul(lua_State *state) {
}

luaL_Reg string_apis[] = {
  {"qnode_strtoul",  qnode_strtoul},
  {NULL, NULL},
};
