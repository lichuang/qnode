/*
 * Copyright (C) codedump
 */

#include "core/log.h"
#include "script/actor.h"
#include "script/key.h"
#include "script/lua_thread.h"
#include "script/lua_util.h"
#include "script/lua_vm.h"

LuaThread::LuaThread(LuaVM *vm, Actor* actor)
  : thread_(vm->NewThread()),
    vm_(vm) {
  lua_State* L = vm->LuaState();

  // move code closure to new coroutine
  lua_xmove(L, thread_, 1);

  // set closure's env table to new coroutine's globals table
  GetGlobalTable(thread_);
  lua_setfenv(thread_, -2);

  SetActor(thread_, actor);
}

LuaThread::~LuaThread() {
}

int
LuaThread::Resume(int nret) {
  int status = lua_resume(thread_, nret);
  if (status != LUA_YIELD) {
    if (status == LUA_ERRRUN && lua_isstring(thread_, -1)) {
      Errorf("LUA_ERRRUN: %s", lua_tostring(thread_, -1));
      lua_pop(thread_, -1);
    }
  }

  return status;
}
