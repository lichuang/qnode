/*
 * Copyright (C) codedump
 */

#include "core/log.h"
#include "script/lua_api.h"
#include "script/lua_util.h"

lua_State*
NewLuaThread(lua_State *L) {
  int        base;
  lua_State* co;

  // save main thread stack
  base = lua_gettop(L);

  // new lua thread
  co = lua_newthread(L);

  // new globals table for coroutine
  CreateNewGlobalTable(co, 0, 0);

  lua_createtable(co, 0, 1);

  GetGlobalTable(co);
  lua_setfield(co, -2, "__index");
  lua_setmetatable(co, -2);
  SetGlobalTable(co);

  /*
  int ref = luaL_ref(L, -2);
  if (ref == LUA_NOREF) {
    lua_settop(L, base);
    return NULL;
  }
  */

  // restore main thread stack
  lua_settop(L, base);

  return co;
}

// Create new table and set _G field to itself.
void
CreateNewGlobalTable(lua_State* L, int narr, int nrec) {
  lua_createtable(L, narr, nrec + 1);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "_G");
}

void
GetGlobalTable(lua_State* L) {
  lua_pushvalue(L, LUA_GLOBALSINDEX);
}

void
SetGlobalTable(lua_State* L) {
  lua_replace(L, LUA_GLOBALSINDEX);
}
