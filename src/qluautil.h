/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QLUAUTIL_H__
#define __QLUAUTIL_H__

#include <lua.h>
#include "qcore.h"
#include "qdict.h"
#include "qlog.h"
#include "qstring.h"

struct qactor_t;
struct qactor_msg_t;
struct qworker_t;

lua_State* qlua_new_state(lua_Alloc fun, void *ud);
lua_State* qlua_new_thread(qworker_t *worker);

int  qlua_reload(lua_State *state);
#define qlua_get_global_table(state, key) qlua_get_table(state, LUA_GLOBALSINDEX, key)

int  qlua_get_table(lua_State *state, int idx, const char *key);
int  qlua_get_table_string(lua_State *state,
                           const char *key, qstring_t *string);
int  qlua_get_table_number(lua_State *state,
                           const char *key, int *number);

int  qlua_call(lua_State *state, int args, int results);

int  qlua_copy_state_table(lua_State *src,
                           lua_State *dst, int table_idx);
int  qlua_copy_table(lua_State *state, int table_idx, qdict_t *dict);
void qlua_dump_dict(lua_State *state, qdict_t *dict);

int  qlua_threadloadfile(qactor_t *actor, lua_State *state,
                         const char *filename);
int  qlua_dofile(lua_State *state, const char *filename);
int  qlua_init_path(lua_State *state);

int  qlua_doresume(lua_State *state, int nargs,
                   const char *file, int line);

qactor_t* qlua_get_actor(lua_State *state);

#define qlua_fail(state) qerror("%s:%d lua_call failed\n\t%s", __FILE__, __LINE__, lua_tostring(state, -1));
#define qlua_resume(state, nargs) qlua_doresume(state, nargs, __FILE__, __LINE__);

#endif  /* __QLUAUTIL_H__ */
