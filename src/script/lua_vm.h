/*
 * Copyright (C) codedump
 */
#ifndef __QNODE_SCRIPT_LUA_VM_H__
#define __QNODE_SCRIPT_LUA_VM_H__

#include <string>
#include <list>
#include "script/lua.h"

using namespace std;

class ScriptIOThread;

class LuaVM {
public:
  LuaVM(const string &main, ScriptIOThread *);

  lua_State* NewThread();
  lua_State* LuaState() {
    return state_;
  }

private:
  void initLuaPath();

private:
  lua_State *state_;

};

#endif  // __QNODE_SCRIPT_LUA_VM_H__
