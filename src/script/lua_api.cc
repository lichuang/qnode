/*
 * Copyright (C) codedump
 */

#include "script/lua_api.h"

extern "C" {

extern void RegisterLuaLogApis(lua_State*);
extern void RegisterSocketApis(lua_State*);
extern void RegisterReqSocketApis(lua_State*);

static void
RegisterReqApis(lua_State *L) {
  // actor.req table
  lua_createtable(L, 0 /* narr */, 24 /* nrec */);

  RegisterReqSocketApis(L);

  lua_setfield(L, -2, "req");
}

void
RegisterLuaApis(lua_State *L) {
  lua_createtable(L, 0 /* narr */, 116 /* nrec */);    /* qactor.* */

  // log apis
  RegisterLuaLogApis(L);

  // socket apis
  RegisterSocketApis(L);

  // req apis
  RegisterReqApis(L);

  lua_getglobal(L, "package"); /* actor package */
  lua_getfield(L, -1, "loaded"); /* actor package loaded */
  lua_pushvalue(L, -3); /* actor package loaded ngx */
  lua_setfield(L, -2, "ngx"); /* actor package loaded */
  lua_pop(L, 2);

  lua_setglobal(L, "actor");
}
};
