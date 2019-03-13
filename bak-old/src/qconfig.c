/*
 * See Copyright Notice in qnode.h
 */

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "qconfig.h"
#include "qlimits.h"
#include "qlog.h"
#include "qluautil.h"
#include "qserver.h"

qconfig_t config;

static void config_init_log(lua_State *state);
static void config_init_worker(lua_State *state);
static void config_init_script(lua_State *state);
static void config_init_server(lua_State *state);
static void config_set_default();

static void
config_init_log(lua_State *state) {
  if (qlua_get_global_table(state, "log") < 0) {
    return;
  }

  qlua_get_table_string(state, "path", &(config.log_path));

  qlua_get_table_string(state, "level", &(config.log_level));

  lua_pop(state, 1);
}

static void
config_init_worker(lua_State *state) {
  if (qlua_get_global_table(state, "worker") < 0) {
    qstdout("not found worker config, use default\n");
    return;
  }

  qlua_get_table_number(state, "num", &(config.worker));
  if (config.worker > QMAX_WORKER) {
    config.worker = QMAX_WORKER;
  }
  lua_pop(state, 1);
}

static void
config_init_script(lua_State *state) {
  if (qlua_get_global_table(state, "script") < 0) {
    qstdout("not found script config, use default\n");
    return;
  }

  if (qlua_get_table_string(state, "path",
                            &(config.script_path)) < 0) {
    qstdout("not found script path config\n");
    exit(-1);
  }

  qlua_get_table_string(state, "main", &(config.main));

  lua_pop(state, 1);
}

static void
config_init_server(lua_State *state) {
  if (qlua_get_global_table(state, "server") < 0) {
    qstdout("not found server config, use default\n");
    return;
  }
  qlua_get_table_number(state, "daemon", &(config.daemon));
  qlua_get_table_number(state, "recycle_internal",
                        &(config.recycle_internal));
  lua_pop(state, 1);
}

static void
config_set_default() {
  config.worker           = 5;
  config.daemon           = 0;
  config.recycle_internal = 60;
  config.script_path      = qstring_new("./script");
  config.main             = qstring_new("main.lua");
  config.log_path         = qstring_new("./log");
  config.log_level        = qstring_new("debug");
  config.log_size         = 1024000000;
  getcwd(config.cwd, 1024);
}

int
qconfig_init(const char *filename) {
  lua_State *state;

  config_set_default();

  state = lua_open();
  if (state == NULL) {
    qstdout("create lua state error\n");
    return QERROR;
  }

  if (luaL_dofile(state, filename) != 0) {
    qstdout("parse config %s error:\n\t%s\n", filename,
            lua_tostring(state, -1));
    return QERROR;
  }

  config_init_worker(state);  
  config_init_script(state);
  config_init_server(state);
  config_init_log(state);

  lua_close(state);

  return QOK;
}

void
qconfig_free() {
  qstring_destroy(config.script_path);
  qstring_destroy(config.log_path);
  qstring_destroy(config.log_level);
  qstring_destroy(config.main);
}
