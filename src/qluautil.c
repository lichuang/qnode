/*
 * See Copyright Notice in qnode.h
 */

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <string.h>
#include "qactor.h"
#include "qassert.h"
#include "qconfig.h"
#include "qdefines.h"
#include "qluautil.h"
#include "qlog.h"
#include "qmsg.h"
#include "qserver.h"
#include "qstring.h"
#include "qthread.h"

static int panic(lua_State *state) {
  qerror("PANIC: unprotected error in call to Lua API (%s)\n", lua_tostring(state, -1));
  return 0;
}

lua_State* qlua_new_state() {
  lua_State *state = lua_open();
  lua_atpanic(state, panic);
  luaL_openlibs(state);
  return state;
}

lua_State* qlua_new_thread(qthread_t *thread) {
  return lua_newthread(thread->state);
}

static int err_func(lua_State * state) {
  lua_getfield(state, LUA_GLOBALSINDEX, "debug");
  if (!lua_istable(state, -1)) {
    lua_pop(state, 1); 
    return 1;
  }

  lua_getfield(state, -1, "traceback");
  if (!lua_isfunction(state, -1)) {
    lua_pop(state, 2); 
    return 1;
  }

  lua_pushvalue(state, 1);  /* pass error message */
  lua_pushinteger(state, 2);  /* skip this function and traceback */
  lua_call(state, 2, 1);  /* call debug.traceback */
  return 1;
}

int qlua_call(lua_State *state, int args, int results) {
  int base = lua_gettop(state) - args;
  lua_pushcfunction(state, &err_func);
  lua_insert(state, base);
  int ret = lua_pcall(state, args, results, base);
  if (lua_status(state) != LUA_YIELD) {
    lua_remove(state, base);
  }
  return ret;
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

void qlua_copy_state_table(lua_State *src, lua_State *dst, int table_idx) {
  lua_newtable(dst);
  lua_pushvalue(dst, 1);
  if (lua_istable(src, table_idx)) {
    lua_pushnil(src);
    size_t len;
    const char *key;
    const char *str_val = NULL;
    double num_val = 0;
    void *user_data = NULL;
    int type;
    while (lua_next(src, table_idx)) {
      int val_idx = lua_gettop(src);
      int key_idx = val_idx - 1;
      type = lua_type(src, val_idx);
      if (type == LUA_TSTRING) {
        str_val = lua_tolstring(src, val_idx, &len);
      } else if (type == LUA_TNUMBER) {
        num_val = lua_tonumber(src, val_idx);
      } else if (type == LUA_TLIGHTUSERDATA) {
        user_data = lua_touserdata(src, val_idx);
      } else {
        qerror("error type: %d", type);
        return;
      }

      key = lua_tolstring(src, key_idx, &len);

      lua_pushstring(dst, key);
      if (str_val) {
        lua_pushstring(dst, str_val);
        str_val = NULL;
      } else if (user_data) {
        lua_pushlightuserdata(dst, user_data);
        user_data = NULL;
      } else {
        lua_pushnumber(dst, num_val);
      }
      lua_rawset(dst, -3);
      lua_pop(src, 1);
    }
  }
}

int qlua_copy_table(lua_State *state, int table_idx, qdict_t *dict) {
  if (lua_istable(state, table_idx)) {
    lua_pushnil(state);
    size_t len;
    const char *key;
    const char *str_val = NULL;
    double num_val = 0;
    int type;
    while (lua_next(state, table_idx)) {
      int val_idx = lua_gettop(state);
      int key_idx = val_idx - 1;
      type = lua_type(state, val_idx);
      if (type == LUA_TSTRING) {
        str_val = lua_tolstring(state, val_idx, &len);
      } else if (type == LUA_TNUMBER) {
        num_val = lua_tonumber(state, val_idx);
      } else {
        qerror("child arg table val MUST be number or string");
        return -1;
      }

      key = lua_tolstring(state, key_idx, &len);
      qkey_t key_val;
      QKEY_STRING(key_val, (char*)key, NULL);

      qval_t val;
      if (str_val) {
        qstring_t *tmp = &(val.data.str);
        qstring_null_set(tmp, dict->pool);
        qstring_assign(tmp, str_val);
      } else {
        QVAL_NUMBER(val, num_val);
      }
      qdict_add(dict, &key_val, &val);
      lua_pop(state, 1);

      str_val = NULL;
    }
  }
  return 0;
}

void qlua_dump_dict(lua_State *state, qdict_t *dict) {
  qdict_iter_t *iter = qdict_iterator(dict);
  qdict_entry_t *entry = NULL;
  while ((entry = qdict_next(iter)) != NULL) {
    qkey_t *key = &(entry->key);
    qval_t *val = &(entry->val);
    if (key->type == QDICT_KEY_STRING) {
      lua_pushstring(state, key->data.str.data);
    } else {
      lua_pushnumber(state, key->data.num);
    }
    if (val->type == QDICT_VAL_STRING) {
      lua_pushstring(state, val->data.str.data);
    } else {
      lua_pushnumber(state, val->data.num);
    }
    lua_settable(state, -3);
  }
}

static void lua_init_filename(const char *filename, qstring_t *full_name) {
  qserver_t *server = g_server;
  //qstring_init_str(*full_name);
  qstring_assign(full_name, server->config->script_path.data);
  qstring_append(full_name, "/");
  qstring_append(full_name, filename);
}

int qlua_threadloadfile(qactor_t *actor, lua_State *state, const char *filename) {
  /* TODO: check the state is a lua thread */
  qstring_t full_name = qstring_null(actor->pool);
  lua_init_filename(filename, &full_name);
  int ret = luaL_loadfile(state, full_name.data);
  qstring_destroy(&full_name);
  // start the coroutine
  lua_resume(state, 0);
  return ret;
}

int qlua_dofile(lua_State *state, const char *filename) {
  qstring_t full_name;
  lua_init_filename(filename, &full_name);
  int ret = luaL_dofile(state, full_name.data);
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
  qstring_null_set(&full_path, actor->pool);
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

struct qactor_t* qlua_get_actor(lua_State *state) {
  //lua_getglobal(state, "qnode");
  lua_pushlightuserdata(state, state);
  lua_gettable(state, LUA_REGISTRYINDEX);
  //lua_getfield(state, LUA_REGISTRYINDEX, "qnode");
  return (qactor_t*)lua_touserdata(state, -1);
}

void qlua_fail(lua_State *state, char *file, int line) {                                           \
  char tmp_buff[8000+1]={0,};                                              
  snprintf(tmp_buff, 8000, "%s:%d lua_call failed\n\t%s",
           file, line, lua_tostring(state, -1 )); 
  qerror(tmp_buff); 
  /*
  lua_State* L_ = g_luasvr->L();                                                  
  assert( L_ );                                                                   
  g_luasvr->get_lua_ref( ADD_LLUA_FAIL_LOG );                                     
  lua_pushstring( L_,  tmp_buff );                                                
  if( llua_call( L_, 1, 0, 0 ) )                                                  
  {                                                                               
    ERR(2)("[LUAWRAPPER](lmisc) %s:%d add llua fail err.", __FILE__, __LINE__ );
  } 
  */
}
