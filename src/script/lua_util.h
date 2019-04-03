/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_SCRIPT_LUA_UTIL_H__
#define __QNODE_SCRIPT_LUA_UTIL_H__

#include "script/lua.h"

class ScriptIOThread;

lua_State* NewLuaThread(lua_State *);

void CreateNewGlobalTable(lua_State*, int narr, int nrec);
void GetGlobalTable(lua_State*);
void SetGlobalTable(lua_State*);

#endif  // __QNODE_SCRIPT_LUA_UTIL_H__
