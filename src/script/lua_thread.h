/*
 * Copyright (C) codedump
 */

#ifndef __SCRIPT_LUA_THREAD_H__
#define __SCRIPT_LUA_THREAD_H__

#include "script/lua.h"

class Actor;
class LuaVM;

class LuaThread {
public:
  LuaThread(LuaVM*, Actor*);
  ~LuaThread();

  int Resume(int nret);
  lua_State* Thread() {
    return thread_;
  }

private:
  lua_State* thread_;
  LuaVM* vm_;
};

#endif  // __SCRIPT_LUA_THREAD_H__
