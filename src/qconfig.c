/*
 * See Copyright Notice in qnode.h
 */

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdio.h>
#include "qconfig.h"
#include "qluautil.h"

static void config_init_thread(qconfig_t *config, lua_State *L) {
  qlua_get_table(L, -1, "thread");
  qlua_get_table_number(L, "num", &(config->thread_num));
  lua_pop(L, 1);
}

static void config_init_script(qconfig_t *config, lua_State *L) {
  qlua_get_table(L, -1, "script");
  qlua_get_table_string(L, "path", &(config->script_path));
  lua_pop(L, 1);
}

static void config_set_default(qconfig_t *config) {
  config->thread_num = 1;
  qstring_init(&(config->script_path));
  qstring_assign(&(config->script_path), "./script");
}

int qconfig_init(qconfig_t *config, const char *filename) {
  config_set_default(config);
  if (filename == NULL) {
    return -1;
  }
  lua_State *L = lua_open();
  if (luaL_dofile(L, filename) != 0) {
    return -1;
  }

  if (qlua_get_global_table(L, "qnode_config") < 0) {
    return -1;
  }

  config_init_thread(config, L);  
  config_init_script(config, L);

  lua_close(L);
  return 0;
}
