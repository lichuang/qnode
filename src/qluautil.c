/*
 * See Copyright Notice in qnode.h
 */

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "qactor.h"
#include "qapi.h"
#include "qassert.h"
#include "qconfig.h"
#include "qdefines.h"
#include "qluautil.h"
#include "qmsg.h"
#include "qserver.h"
#include "qstring.h"
#include "qworker.h"

static int
panic(lua_State *state) {
  /*
  qerror("PANIC: unprotected error in call to Lua API (%s)\n", lua_tostring(state, -1));
  return 0;
  */
  int       i;
  lua_Debug ldb;
  for(i = 0; lua_getstack(state, i, &ldb ) == 1; i++ ) { 
    lua_getinfo(state, ">Slnu", &ldb );
    const char *name = ldb.name ? ldb.name : ""; 
    const char *filename = ldb.source ? ldb.source : ""; 
    qerror("%s '%s' @ file '%s', line %d", ldb.what,
           name, filename, ldb.currentline);
  } 

  return 0;
}

lua_State*
qlua_new_state(lua_Alloc fun, void *ud) {
  lua_State *state;

  state = lua_newstate(fun, ud);
  if (state == NULL) {
    qstdout("lua_newstate error\n");
    return NULL;
  }

  lua_atpanic(state, panic);
  luaL_openlibs(state);
  qapi_register(state);
  if (qlua_init_path(state) != QOK) {
    qstdout("qlua_init_path error\n");
    lua_close(state);
    return NULL;
  }

  if(luaL_dofile(state, config.main)) {
    qerror("load main script %s error\n", config.main);
    qlua_fail(state);
    lua_close(state);
    return NULL;
  }

  return state;
}

int
qlua_reload(lua_State *state) {
  if(luaL_dofile(state, config.main)) {
    qerror("reload main script %s error\n", config.main);
    qlua_fail(state);
    return QERROR;
  }

  return QOK;
}

lua_State*
qlua_new_thread(qworker_t *worker) {
  lua_State *state;
  
  state = lua_newthread(worker->state);
  if (state == NULL) {
    return NULL;
  }

  return state;
}

static int
err_func(lua_State * state) {
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

int
qlua_call(lua_State *state, int args, int results) {
  int base, ret;

  base = lua_gettop(state) - args;
  lua_pushcfunction(state, &err_func);
  lua_insert(state, base);
  ret = lua_pcall(state, args, results, base);
  if (lua_status(state) != LUA_YIELD) {
    lua_remove(state, base);
  }
  return ret;
}

int
qlua_get_table(lua_State *state, int idx, const char *key) {
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

int
qlua_get_table_string(lua_State *state, const char *key,
                      qstring_t *string) {
  int ret;

  lua_pushvalue(state, -1);
  lua_pushstring(state, key);
  lua_gettable(state, -2);
  if(!lua_isnil(state, -1)) {
    *string = qstring_assign(*string, lua_tostring(state, -1));
    ret = 0;
  } else {
    ret = -1;
  }
  lua_pop(state, 2);
  return ret;
}

int
qlua_get_table_number(lua_State *state, const char *key, int *number) {
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

int
qlua_copy_state_table(lua_State *src, lua_State *dst,
                      int table_idx) {
  int         type;
  int         val_idx;
  int         key_idx;
  size_t      len;
  double      num_val;
  const char *key;
  const char *str_val;
  void       *user_data;

  lua_newtable(dst);
  lua_pushvalue(dst, 1);
  if (lua_istable(src, table_idx)) {
    lua_pushnil(src);
    str_val = NULL;
    num_val = 0;
    user_data = NULL;
    while (lua_next(src, table_idx)) {
      val_idx = lua_gettop(src);
      key_idx = val_idx - 1;
      type = lua_type(src, val_idx);
      if (type == LUA_TSTRING) {
        str_val = lua_tolstring(src, val_idx, &len);
      } else if (type == LUA_TNUMBER) {
        num_val = lua_tonumber(src, val_idx);
      } else if (type == LUA_TLIGHTUSERDATA) {
        user_data = lua_touserdata(src, val_idx);
      } else {
        qerror("error type: %d", type);
        return QERROR;
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

  return QOK;
}

int
qlua_copy_table(lua_State *state, int table_idx,
                qdict_t *dict) {
  int         type;
  int         val_idx;
  int         key_idx;
  double      num_val;
  size_t      len;
  const char *key;
  const char *str_val;

  if (lua_istable(state, table_idx)) {
    lua_pushnil(state);
    str_val = NULL;
    num_val = 0;
    while (lua_next(state, table_idx)) {
      val_idx = lua_gettop(state);
      key_idx = val_idx - 1;
      type = lua_type(state, val_idx);
      if (type == LUA_TSTRING) {
        str_val = lua_tolstring(state, val_idx, &len);
      } else if (type == LUA_TNUMBER) {
        num_val = lua_tonumber(state, val_idx);
      } else {
        qerror("child arg table val MUST be number or string");
        return QERROR;
      }

      key = lua_tolstring(state, key_idx, &len);

      if (str_val) {
        if (qdict_setstr(dict, key, str_val) == NULL) {
          return QERROR;
        }
      } else {
        if (qdict_setnum(dict, key, num_val) == NULL) {
          return QERROR;
        }
      }
      lua_pop(state, 1);

      str_val = NULL;
    }
  }
  return QOK;
}

void
qlua_dump_dict(lua_State *state, qdict_t *dict) {
  qdict_iter_t  iter = qdict_iter(dict);
  qstring_t     key;
  qvalue_t     *val;
  qdict_node_t *node;

  while ((node = qdict_next(&iter)) != NULL) {
    key = node->key;
    val = &(node->value);

    lua_pushstring(state, key);
    if (qvalue_isstring(val)) {
      lua_pushstring(state, val->data.str);
    } else {
      lua_pushnumber(state, val->data.num);
    }
    lua_settable(state, -3);
  }
}

static qstring_t
lua_init_filename(const char *filename) {
  qstring_t full_name;

  full_name = qstring_new(config.script_path);
  if (!full_name) {
    return NULL;
  }
  full_name = qstring_append(full_name, "/");
  if (!full_name) {
    return NULL;
  }
  full_name = qstring_append(full_name, filename);

  return full_name;
}

int
qlua_threadloadfile(qactor_t *actor, lua_State *state,
                    const char *filename) {
  int       ret;
  qstring_t full_name;

  UNUSED(actor);

  /* TODO: check the state is a lua thread */
  full_name = (qstring_t)filename;
  if (full_name == NULL) {
    return -1;
  }
  ret = luaL_loadfile(state, full_name);
  /* start the coroutine */
  qlua_resume(state, 0);

  return ret;
}

int
qlua_dofile(lua_State *state, const char *filename) {
  int       ret;
  qstring_t full_name;

  full_name = lua_init_filename(filename);
  if (full_name == NULL) {
    return -1;
  }
  ret = luaL_dofile(state, full_name);
  qstring_destroy(full_name);

  return ret;
}

int
qlua_init_path(lua_State *state) {
  const char *cur_path;
  qstring_t   full_path;

  lua_getglobal(state, "package");
  lua_getfield(state, -1, "path");
  cur_path = lua_tostring(state, -1);
  full_path = qstring_new(cur_path);
  if (full_path == NULL) {
    return QERROR;
  }

  full_path = qstring_catvprintf(full_path, ";%s/script/?.lua",
                                 config.cwd);
  if (full_path == NULL) {
    return QERROR;
  }

  lua_pop(state, 1);
  lua_pushstring(state, full_path);
  lua_setfield(state, -2, "path");
  lua_pop(state, 1);
  qstring_destroy(full_path);

  return QOK;
}

qactor_t*
qlua_get_actor(lua_State *state) {
  lua_pushlightuserdata(state, state);
  lua_gettable(state, LUA_REGISTRYINDEX);
  return (qactor_t*)lua_touserdata(state, -1);
}

int
qlua_doresume(lua_State *state, int nargs,
              const char *file, int line) {
  int         ret;

  ret = lua_resume(state, nargs);

  /* if an error occured */
  if (ret > LUA_YIELD) {
    qerror("%s:%d lua_call failed\n\t%s", __FILE__, __LINE__,
           lua_tostring(state, -1));
    /* TODO: when error occur in coroutine, it cannot continue... */
  }

  return ret;
}
