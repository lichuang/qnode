/*
 * See Copyright Notice in qnode.h
 */

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "qconfig.h"
#include "qlog.h"
#include "qluautil.h"
#include "qserver.h"

static void config_init_log(qconfig_t *config, lua_State *state);
static void config_init_worker(qconfig_t *config, lua_State *state);
static void config_init_script(qconfig_t *config, lua_State *state);
static void config_init_server(qconfig_t *config, lua_State *state);
static void config_set_default(qconfig_t *config);

static void
config_init_log(qconfig_t *config, lua_State *state) {
  qlua_get_global_table(state, "qnode_config");
  if (qlua_get_table(state, -1, "log") < 0) {
    qstdout("not found log config, use default\n");
    return;
  }
  if (qlua_get_table_string(state, "path", &(config->log_path)) < 0) {
    qstdout("read log[path] config error\n");
    exit(-1);
  }
  if (qlua_get_table_string(state, "level", &(config->log_level)) < 0) {
    qstdout("read log[level] config error\n");
    exit(-1);
  }
  lua_pop(state, 1);
}

static void
config_init_worker(qconfig_t *config, lua_State *state) {
  qlua_get_global_table(state, "qnode_config");
  if (qlua_get_table(state, -1, "worker") < 0) {
    qstdout("not found worker config, use default\n");
    return;
  }
  qlua_get_table_number(state, "num", &(config->worker));
  lua_pop(state, 1);
}

static void
config_init_script(qconfig_t *config, lua_State *state) {
  qlua_get_global_table(state, "qnode_config");
  if (qlua_get_table(state, -1, "script") < 0) {
    qstdout("not found script config, use default\n");
    return;
  }
  if (qlua_get_table_string(state, "path", &(config->script_path)) < 0) {
    qstdout("read script[path] config error\n");
    exit(-1);
  }
  lua_pop(state, 1);
}

static void
config_init_server(qconfig_t *config, lua_State *state) {
  qlua_get_global_table(state, "qnode_config");
  if (qlua_get_table(state, -1, "server") < 0) {
    qstdout("not found server config, use default\n");
    return;
  }
  qlua_get_table_number(state, "daemon", &(config->daemon));
  lua_pop(state, 1);
}

static void
config_set_default(qconfig_t *config) {
  config->worker  = 5;
  config->daemon      = 0;
  config->script_path = qstring_new("./script");
  config->log_path    = qstring_new("./log");
  config->log_level   = qstring_new("debug");
  config->log_size    = 1024000000;
}

int
qconfig_init(qconfig_t *config, const char *filename) {
  lua_State *state;

  config_set_default(config);
  if (filename == NULL) {
    return -1;
  }

  state = lua_open();
  if (luaL_dofile(state, filename) != 0) {
    printf("open config %s error\n", filename);
    return -1;
  }

  if (qlua_get_global_table(state, "qnode_config") < 0) {
    qstdout("table qnode_config not exist in config, use default config\n");
    lua_close(state);
    return 0;
  } else {
    lua_pop(state, 1);
  }

  config_init_worker(config, state);  
  config_init_script(config, state);
  config_init_server(config, state);
  config_init_log(config, state);

  lua_close(state);
  return 0;
}

void
qconfig_free(qconfig_t *config) {
  qstring_destroy(config->script_path);
  qstring_destroy(config->log_path);
  qstring_destroy(config->log_level);
}
