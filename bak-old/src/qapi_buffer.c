/*
 * See Copyright Notice in qnode.h
 */

#include <lauxlib.h>
#include <string.h>
#include "qbuffer.h"
#include "qluautil.h"
#include "qlog.h"

static int get_buffer_len(lua_State *state, int read);

static int qlbuffer_find(lua_State *state);
static int qlbuffer_write_string(lua_State *state);
static int qlbuffer_write_char(lua_State *state);
static int qlbuffer_set(lua_State *state);
static int qlbuffer_get(lua_State *state);
static int qlbuffer_reset(lua_State *state);
static int qlbuffer_rlen(lua_State *state);
static int qlbuffer_wlen(lua_State *state);

luaL_Reg buffer_apis[] = {
  {"qlbuffer_find",          qlbuffer_find},
  {"qlbuffer_rlen",          qlbuffer_rlen},
  {"qlbuffer_wlen",          qlbuffer_wlen},

  {"qlbuffer_write_string",  qlbuffer_write_string},
  {"qlbuffer_write_char",    qlbuffer_write_char},

  {"qlbuffer_reset",         qlbuffer_reset},
  {"qlbuffer_set",           qlbuffer_set},
  {"qlbuffer_get",           qlbuffer_get},

  {NULL, NULL},
};

static int
qlbuffer_find(lua_State *state) {
  int         pos;
  char       *ret;
  const char *str;
  qbuffer_t  *buffer;

  buffer = (qbuffer_t*)lua_touserdata(state, 1);
  pos = (int)lua_tonumber(state, 2);
  str = lua_tostring(state, 3);

  if (buffer == NULL || str == NULL || pos < 0) {
    lua_pushnil(state);
    lua_pushliteral(state, "wrong param");
    return 2;
  }

  ret = strstr(buffer->data + pos, str);
  if (ret == NULL) {
    lua_pushnumber(state, -1);
  } else {
    lua_pushnumber(state, ret - buffer->data);
  }

  return 1;
}

static int
qlbuffer_write_string(lua_State *state) {
  const char *str;
  qbuffer_t  *buffer;

  buffer = (qbuffer_t*)lua_touserdata(state, 1);
  str = lua_tostring(state, 2);
  if (buffer == NULL || str == NULL) {
    lua_pushnil(state);
    lua_pushliteral(state, "wrong param");
    return 2;
  }

  if (qbuffer_write(buffer, str, strlen(str)) < 0) {
    lua_pushnil(state);
    lua_pushliteral(state, "write error");
    return 2;
  }

  lua_pushliteral(state, "OK");
  return 1; 
}

static int
qlbuffer_write_char(lua_State *state) {
  const char *str;
  qbuffer_t  *buffer;

  buffer = (qbuffer_t*)lua_touserdata(state, 1);
  str    = lua_tostring(state, 2);

  if (buffer == NULL || str == NULL) {
    lua_pushnil(state);
    lua_pushliteral(state, "wrong param");
    return 2;
  }

  if (qbuffer_write(buffer, &str[0], 1) < 0) {
    lua_pushnil(state);
    lua_pushliteral(state, "write error");
    return 1;
  }

  lua_pushliteral(state, "OK");
  return 1; 
}

static int
qlbuffer_set(lua_State *state) {
  int         pos, len;
  const char *str;
  qbuffer_t  *buffer;

  buffer = (qbuffer_t*)lua_touserdata(state, 1);
  pos = (int)lua_tonumber(state, 2);
  str = lua_tostring(state, 3);

  if (buffer == NULL || str == NULL || pos < 0) {
    lua_pushnil(state);
    lua_pushliteral(state, "wrong param");
    return 2;
  }
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
qlbuffer_get(lua_State *state) {
  qbuffer_t  *buffer;
  int         pos, len, buflen;

  buffer = (qbuffer_t*)lua_touserdata(state, 1);
  pos = (int)lua_tonumber(state, 2);
  len = (int)lua_tonumber(state, 3);

  if (buffer == NULL || pos < 0 || len < 0) {
    lua_pushnil(state);
    lua_pushliteral(state, "wrong param");
    return 2;
  }

  buflen = strlen(buffer->data + pos);
  if (len == 0 || len > buflen) {
    len = buflen;
  }
  lua_pushlstring(state, buffer->data + pos, len);

  return 1;
}

static int
qlbuffer_reset(lua_State *state) {
  qbuffer_t  *buffer;

  buffer = (qbuffer_t*)lua_touserdata(state, 1);
  if (buffer == NULL) {
    lua_pushnil(state);
    lua_pushliteral(state, "wrong param");
    return 2;
  }

  qbuffer_reset(buffer);

  lua_pushliteral(state, "OK");
  return 1; 
}

static int
get_buffer_len(lua_State *state, int read) {
  qbuffer_t  *buffer;

  buffer = (qbuffer_t*)lua_touserdata(state, 1);
  if (buffer == NULL) {
    lua_pushnil(state);
    lua_pushliteral(state, "wrong param");
    return 2;
  }

  if (read) {
    lua_pushnumber(state, qbuffer_rlen(buffer));
  } else {
    lua_pushnumber(state, qbuffer_wlen(buffer));
  }

  return 1;
}

static int
qlbuffer_rlen(lua_State *state) {
  return get_buffer_len(state, 1);
}

static int
qlbuffer_wlen(lua_State *state) {
  return get_buffer_len(state, 0);
}
