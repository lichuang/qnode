/*
 * See Copyright Notice in qnode.h
 */

#include "qapi.h"
#include "qluautil.h"
#include "qworker.h"

extern luaL_Reg buffer_apis[];
extern luaL_Reg log_apis[];
extern luaL_Reg net_apis[];
extern luaL_Reg node_apis[];
extern luaL_Reg string_apis[];

typedef luaL_Reg* api_array;

static api_array array[] = {
  &buffer_apis[0],
  &log_apis[0],
  &net_apis[0],
  &node_apis[0],
  &string_apis[0],
  NULL
};

void
qapi_register(lua_State *state, qactor_t *actor) {
  int i, j;

  for (i = 0; array[i] != NULL; ++i) {
    for (j = 0; array[i][j].name != NULL; ++j) {
      const char *name = array[i][j].name;
      lua_CFunction func = array[i][j].func;
      lua_register(state, name, func);
    }
  }

  /*
  qthread_t *thread = qactor_get_thread(actor);
  lua_State *thread_state = thread->state;
  lua_pushvalue(thread_state, LUA_GLOBALSINDEX);
  */

#if 0
  lua_createtable(state, 0, 1 + 1);
  lua_pushvalue(state, -1);
  lua_setfield(state, -2, "_G");

  lua_createtable(state, 0, 1);
  //qlua_copy_state_table(thread_state, state, 1);
  lua_pushvalue(state, LUA_GLOBALSINDEX);
  lua_setfield(state, -2, "__index");
  lua_setmetatable(state, -2);    /*  setmetatable(newt, {__index = _G}) */
  lua_setfenv(state, -2);    /*  set new running env for the code closure */
#endif

  /*
  lua_pushlightuserdata(state, actor);
  lua_setglobal(state, "qnode");
  */

  /* register the actor-state table */
  lua_pushlightuserdata(state, state);
  lua_pushlightuserdata(state, actor);
  lua_settable(state, LUA_REGISTRYINDEX);
  //lua_setfield(state, LUA_REGISTRYINDEX, "qnode");
}

void
qregister(lua_State *state) {
  int i, j;

  for (i = 0; array[i] != NULL; ++i) {
    for (j = 0; array[i][j].name != NULL; ++j) {
      const char *name = array[i][j].name;
      lua_CFunction func = array[i][j].func;
      lua_register(state, name, func);
    }
  }
}
