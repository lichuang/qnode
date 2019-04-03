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

  // save actor in coroutine global table
  lua_pushlightuserdata(thread_, actor);
  lua_setglobal(thread_, ACTOR_KEY);
}

LuaThread::~LuaThread() {
}

int
LuaThread::Resume(int nret) {
  return lua_resume(thread_, nret);
}
