/*
 * Copyright (C) codedump
 */
#ifndef __QNODE_SCRIPT_LUA_LOG_API_H__
#define __QNODE_SCRIPT_LUA_LOG_API_H__

#include "core/log.h"
#include "script/lua.h"

extern "C" {

static int
luaLog(lua_State *state) {
  const char* str;

  str = lua_tostring(state, 1);
  if (str == NULL) {
    return -1;
  }
  Infof(str);
  return 0;
}

void
RegisterLuaLogApis(lua_State* L) {
  lua_pushcfunction(L, luaLog);
  lua_setfield(L, -2, "log");
}

};

#endif  // __QNODE_SCRIPT_LUA_LOG_API_H__
