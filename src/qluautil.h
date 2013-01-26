/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QLUAUTIL_H__
#define __QLUAUTIL_H__

#include <lua.h>
#include "qstring.h"

struct qactor_t;
struct qactor_msg_t;
struct qthread_t;

lua_State* qlua_new_state();
lua_State* qlua_new_thread(struct qthread_t *thread);

#define qlua_get_global_table(state, key) qlua_get_table(state, LUA_GLOBALSINDEX, key)

int qlua_get_table(lua_State *state, int idx, const char *key);
int qlua_get_table_string(lua_State *state, const char *key, qstring_t *string);
int qlua_get_table_number(lua_State *state, const char *key, int *number);

void qlua_copy_table(lua_State *src, lua_State *dst, int table_idx);
struct qactor_msg_t* qlua_copy_arg_table(lua_State *state, int table_idx);

int qlua_threadloadfile(lua_State *state, const char *filename);
int qlua_dofile(lua_State *state, const char *filename);
int qlua_init_path(struct qactor_t *actor);

struct qactor_t* qlua_get_actor(lua_State *state);

#endif  /* __QLUAUTIL_H__ */
