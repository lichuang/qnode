/*
 * See Copyright Notice in qnode.h
 */

#include <lauxlib.h>
#include <string.h>
#include "qbuffer.h"
#include "qluautil.h"

static int qnode_buffer_find(lua_State *state) {
  qbuffer_t *buffer = (qbuffer_t*)lua_touserdata(state, 1);
  const char *str = lua_tostring(state, 2);
  char *pos = strstr(buffer->data, str);
  if (pos == NULL) {
    lua_pushnumber(state, -1);
  } else {
    lua_pushnumber(state, pos - buffer->data);
  }
  return 1;
}

luaL_Reg buffer_apis[] = {
  {"qnode_buffer_find",    qnode_buffer_find},
  {NULL, NULL},
};
