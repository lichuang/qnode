/*
 * Copyright (C) codedump
 */
#ifndef __QNODE_SCRIPT_LUA_LOG_H__
#define __QNODE_SCRIPT_LUA_LOG_H__

#include "core/log.h"
#include "script/lua.h"

extern "C" {

static int
LuaInfof(lua_State *state) {
  const char* str;

  str = lua_tostring(state, 1);
  if (str == NULL) {
    return -1;
  }
  Infof(str);
  return 0;
}

static int
LuaErrorf(lua_State *state) {
  return 0;
}

luaL_Reg kLogApis[] = {
  {"Infof",   LuaInfof},
  {"Errorf",  LuaErrorf},
  {NULL, NULL},
};

};

#endif  // __QNODE_SCRIPT_LUA_LOG_H__
