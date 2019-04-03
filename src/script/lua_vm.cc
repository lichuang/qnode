/*
 * Copyright (C) codedump
 */

#include <stdlib.h>
#include "base/string.h"
#include "core/log.h"
#include "script/lua_api.h"
#include "script/lua_util.h"
#include "script/lua_vm.h"
#include "script/script_io_thread.h"

static
void* alloc(void *ud, void *ptr, size_t osize, size_t nsize) {
  ScriptIOThread *worker = static_cast<ScriptIOThread*>(ud);
  worker->AddAllocSize(nsize - osize);
  if (nsize == 0) {
    free(ptr);
    return NULL;
  }

  return realloc(ptr, nsize);
}

LuaVM::LuaVM(const string& main, ScriptIOThread *worker)
  : state_(NULL) {
  state_ = lua_newstate(alloc, worker);
  //state_ = luaL_newstate();

  if (state_ == NULL) {
    Fatalf("lua_newstate fail");
    return;
  }

  // open lua libs
  luaL_openlibs(state_);

  // regist all C APIs to Lua
  RegisterLuaApis(state_);

  // init script path
  initLuaPath();

  //if(luaL_dofile(state_, "/home/nemo/source/qnode/lua/init.lua")) {
  if(luaL_loadfile(state_, "/home/nemo/source/qnode/lua/init.lua")) {
    Fatalf("load main script %s error", "config.main");
    lua_close(state_);
  }
}

void
LuaVM::initLuaPath() {
  const char *cur_path;
  string full_path;

  lua_getglobal(state_, "package");
  lua_getfield(state_, -1, "path");
  cur_path = lua_tostring(state_, -1);

  Infof("lua cur_path: %s", cur_path);
  Stringf(&full_path, "%s;%s/?.lua", cur_path, "/home/nemo/source/qnode/lua");

  lua_pop(state_, 1);
  lua_pushstring(state_, full_path.c_str());
  lua_setfield(state_, -2, "path");
  lua_pop(state_, 1);
}

lua_State*
LuaVM::NewThread() {
  return NewLuaThread(state_);
}
