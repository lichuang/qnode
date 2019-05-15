/*
 * Copyright (C) codedump
 */

#include "core/log.h"
#include "script/lua.h"
#include "script/lua_util.h"
#include "script/key.h"

extern "C" {

class Actor;

static int
actorTcpSocket(lua_State *L) {
  Actor *a;

  a = GetActor(L);
  if (a == NULL) {
    return luaL_error(L, "no actor found");
  }

  // construct table with tcp socket meta table
  lua_createtable(L, 5 /* narr */, 1 /* nrec */);
  lua_pushlightuserdata(L, &kTcpSocketMetatableKey);
  lua_rawget(L, LUA_REGISTRYINDEX);
  lua_setmetatable(L, -2);

  return 1;
}

static int
tcpReceive(lua_State *L) {
  Infof("in tcp receive");
  return 0;
}

static int
reqSocket(lua_State *L) {
  Actor *a;

  a = GetActor(L);
  if (a == NULL) {
    return luaL_error(L, "no actor found");
  }

  // save the tcp socket meta table into table
  lua_createtable(L, 3 /* narr */, 1 /* nrec */);
  lua_pushlightuserdata(L, &kTcpSocketMetatableKey);
  lua_rawget(L, LUA_REGISTRYINDEX);
  lua_setmetatable(L, -2);

  lua_settop(L, 1);
  return 1;
}

void
RegisterReqSocketApis(lua_State* L) {
  lua_pushcfunction(L, reqSocket);
  lua_setfield(L, -2, "socket");
}

void
RegisterSocketApis(lua_State* L) {
  lua_createtable(L, 0, 4 /* nrec */);    // actor.socket

  lua_pushcfunction(L, actorTcpSocket);
  lua_pushvalue(L, -1);
  lua_setfield(L, -3, "tcp");
  lua_setfield(L, -2, "stream");

  lua_setfield(L, -2, "socket");

  // raw req socket object metatable
  lua_pushlightuserdata(L, &kTcpSocketMetatableKey);
  lua_createtable(L, 0 /* narr */, 6 /* nrec */);

  lua_pushcfunction(L, tcpReceive);
  lua_setfield(L, -2, "receive");

  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_rawset(L, LUA_REGISTRYINDEX);
}

};
