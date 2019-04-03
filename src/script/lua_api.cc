/*
 * Copyright (C) codedump
 */

#include "script/lua_api.h"

extern "C" {
extern luaL_Reg kLogApis[];

static const luaL_Reg* kLuaApis[] = {
  &kLogApis[0],
  NULL    
};


void
RegisterLuaApis(lua_State *state) {
  int i, j;
  const char *name;

  for (i = 0; kLuaApis[i] != NULL; ++i) {
    for (j = 0; kLuaApis[i][j].name != NULL; ++j) {
      name = kLuaApis[i][j].name;
      lua_CFunction func = kLuaApis[i][j].func;
      lua_register(state, name, func);
    }
  }
}
};
