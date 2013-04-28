/*
 * See Copyright Notice in qnode.h
 */

#include <lauxlib.h>
#include <string.h>
#include "qbuffer.h"
#include "qluautil.h"
#include "qmempool.h"

static int qnode_buffer_find(lua_State *state) {
  int         pos;
  qbuffer_t  *buffer;
  const char *str;
  char       *ret;

  buffer = (qbuffer_t*)lua_touserdata(state, 1);
  pos = (int)lua_tonumber(state, 2);
  str = lua_tostring(state, 3);
  ret = strstr(buffer->data + pos, str);
  if (ret == NULL) {
    lua_pushnumber(state, -1);
  } else {
    lua_pushnumber(state, ret - buffer->data);
  }

  return 1;
}

static int qnode_buffer_set(lua_State *state) {
  int         pos, len;
  const char *str;
  qbuffer_t  *buffer;

  buffer = (qbuffer_t*)lua_touserdata(state, 1);
  pos = (int)lua_tonumber(state, 2);
  str = lua_tostring(state, 3);
  len = strlen(str);
  if (len > (int)(buffer->len - pos)) {
    lua_pushliteral(state, "length to long");
    return 1;
  }
  
  if (len > 0) {
    strncpy(buffer->data + pos, str, len);
  } else {
    buffer->data[pos] = '\0';
  }

  return 0;
}

static int qnode_buffer_get(lua_State *state) {
  qbuffer_t  *buffer;
  int         pos, len;

  buffer = (qbuffer_t*)lua_touserdata(state, 1);
  pos = (int)lua_tonumber(state, 2);
  len = (int)lua_tonumber(state, 3);
  /*
  if (len == -1) {
    len = strlen(buffer->data + pos);
  }
  char *str = qalloc(buffer->pool, len + 1);
  if (str == NULL) {
    lua_pushnil(state);
    lua_pushliteral(state, "no memory");
    return 2;
  }
  strncpy(str, buffer->data + pos, len);
  str[len + 1] = '\0';
  */
  lua_pushlstring(state, buffer->data + pos, len);
  return 1;
}

luaL_Reg buffer_apis[] = {
  {"qnode_buffer_find",    qnode_buffer_find},
  {"qnode_buffer_set",     qnode_buffer_set},
  {"qnode_buffer_get",     qnode_buffer_get},
  {NULL, NULL},
};
