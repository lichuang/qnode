/*
 * See Copyright Notice in qnode.h
 */

#include <lauxlib.h>
#include <string.h>
#include "qbuffer.h"
#include "qluautil.h"
#include "qlog.h"

static int
buffer_find(lua_State *state) {
  int         pos;
  char       *ret;
  const char *str;
  qbuffer_t  *buffer;

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

static int
buffer_write_string(lua_State *state) {
  const char *str;
  qbuffer_t  *buffer;

  buffer = (qbuffer_t*)lua_touserdata(state, 1);
  if (buffer == NULL) {
    lua_pushliteral(state, "buffer NIL");
    return 1;
  }
  str = lua_tostring(state, 2);
  if (str == NULL) {
    lua_pushliteral(state, "str NULL");
    return 1;
  }
  if (qbuffer_write(buffer, str, strlen(str)) < 0) {
    lua_pushliteral(state, "write error");
    return 1;
  }

  lua_pushliteral(state, "OK");
  return 1; 
}

static int
buffer_write_char(lua_State *state) {
  const char *str;
  qbuffer_t  *buffer;

  buffer = (qbuffer_t*)lua_touserdata(state, 1);
  if (buffer == NULL) {
    lua_pushliteral(state, "buffer NIL");
    return 1;
  }
  str = lua_tostring(state, 2);
  if (str == NULL) {
    lua_pushliteral(state, "str NULL");
    return 1;
  }
  if (qbuffer_write(buffer, &str[0], 1) < 0) {
    lua_pushliteral(state, "write error");
    return 1;
  }

  lua_pushliteral(state, "OK");
  return 1; 
}

static int
buffer_set(lua_State *state) {
  int         pos, len;
  const char *str;
  qbuffer_t  *buffer;

  buffer = (qbuffer_t*)lua_touserdata(state, 1);
  if (buffer == NULL) {
    lua_pushliteral(state, "buffer NIL");
    return 1;
  }
  pos = (int)lua_tonumber(state, 2);
  str = lua_tostring(state, 3);
  len = strlen(str);
  qbuffer_reserve(buffer, len);
  
  if (len > 0) {
    strncpy(buffer->data + pos, str, len);
  } else {
    buffer->data[pos] = '\0';
  }

  lua_pushliteral(state, "OK");
  return 1; 
}

static int
buffer_get(lua_State *state) {
  qbuffer_t  *buffer;
  int         pos, len;

  buffer = (qbuffer_t*)lua_touserdata(state, 1);
  pos = (int)lua_tonumber(state, 2);
  len = (int)lua_tonumber(state, 3);
  if (len == 0) {
    len = strlen(buffer->data + pos);
  }
  lua_pushlstring(state, buffer->data + pos, len);

  return 1;
}

static int
buffer_reset(lua_State *state) {
  qbuffer_t  *buffer;

  buffer = (qbuffer_t*)lua_touserdata(state, 1);
  if (buffer) {
    qbuffer_reset(buffer);
  }

  return 0;
}

static int
buffer_length(lua_State *state) {
  qbuffer_t  *buffer;

  buffer = (qbuffer_t*)lua_touserdata(state, 1);
  if (buffer == NULL) {
    return 0;
  }
  lua_pushnumber(state, qbuffer_length(buffer));

  return 1;
}

luaL_Reg buffer_apis[] = {
  {"qbuffer_find",          buffer_find},
  {"qbuffer_length",        buffer_length},

  {"qbuffer_write_string",  buffer_write_string},
  {"qbuffer_write_char",    buffer_write_char},

  {"qbuffer_reset",         buffer_reset},
  {"qbuffer_set",           buffer_set},
  {"qbuffer_get",           buffer_get},

  {NULL, NULL},
};
