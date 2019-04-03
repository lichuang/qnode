/*
 * Copyright (C) codedump
 */

#ifndef __QCORE_SCRIPT_LUA_API_H__
#define __QCORE_SCRIPT_LUA_API_H__

#include "script/lua.h"

extern "C" {
// regist all C APIs to Lua
extern void RegisterLuaApis(lua_State *);
};

#endif  // __QCORE_SCRIPT_LUA_API_H__
