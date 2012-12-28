/*
 * See Copyright Notice in qnode.h
 */

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <string.h>
#include "qactor.h"
#include "qconfig.h"
#include "qluautil.h"
#include "qlog.h"
#include "qserver.h"
#include "qstring.h"
#include "qthread.h"

static int my_panic(lua_State *state) {
  qerror("PANIC: unprotected error in call to Lua API (%s)\n", lua_tostring(state, -1));
  return 0;
}

lua_State* qlua_new_state() {
  lua_State *state = lua_open();
  lua_atpanic(state, my_panic);
  luaL_openlibs(state);
  return state;
}

int qlua_get_table(lua_State *state, int idx, const char *key) {
  lua_pushvalue(state, idx);
  lua_pushstring(state, key);
  lua_gettable(state, -2);

  if (lua_isnil(state, -1) || !lua_istable(state, -1)) { 
    lua_pop(state, 2);
    return -1;
  } else {   
    lua_insert(state, -2);
    lua_pop(state, 1);
    return lua_gettop(state);
  }
}

int qlua_get_table_string(lua_State *state, const char *key, qstring_t *string) {
  lua_pushvalue(state, -1);
  lua_pushstring(state, key);
  lua_gettable(state, -2);
  if(!lua_isnil(state, -1)) {
    qstring_assign(string, lua_tostring(state, -1));
  }
  lua_pop(state, 2);
  return 0;
}

int qlua_get_table_number(lua_State *state, const char *key, int *number) {
  lua_pushvalue(state, -1);
  lua_pushstring(state, key);
  lua_gettable(state, -2);
  if(!lua_isnil(state, -1) && lua_isnumber(state, -1)) {
    *number = (int)lua_tonumber(state, -1);
  } else {
    return -1;
  }
  lua_pop(state, 1);
  return 0;
} 

static void lua_init_filename(const char *filename, qstring_t *full_name) {
  qserver_t *server = g_server;
  qstring_init(full_name);
  qstring_assign(full_name, server->config->script_path.data);
  qstring_append(full_name, "/");
  qstring_append(full_name, filename);
}

int qlua_loadfile(struct qactor_t *actor, const char *filename) {
  qstring_t full_name;
  lua_init_filename(filename, &full_name);
  int ret = luaL_loadfile(actor->state, full_name.data);
  qstring_destroy(&full_name);
  return ret;
}

int qlua_dofile(struct qactor_t *actor, const char *filename) {
  qstring_t full_name;
  lua_init_filename(filename, &full_name);
  int ret = luaL_dofile(actor->state, full_name.data);
  qstring_destroy(&full_name);
  return ret;
}

int qlua_init_path(struct qactor_t *actor) {
  lua_State *state = actor->state;
  qserver_t *server = g_server;
  const char *path = server->config->script_path.data;
  lua_getglobal(state, "package" );
  lua_getfield(state, -1, "path" );
  const char* cur_path = lua_tostring( state, -1 );
  qstring_t full_path;
  qstring_init(&full_path);
  qstring_assign(&full_path, cur_path);
  qstring_append(&full_path, ";");
  qstring_append(&full_path, path);
  qstring_append(&full_path, "/?.lua");
  lua_pop(state, 1);
  lua_pushstring(state, full_path.data);
  lua_setfield(state, -2, "path");
  lua_pop(state, 1);
  qstring_destroy(&full_path);
  return 0;
}
