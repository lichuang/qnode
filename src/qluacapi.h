/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QLUACAPI_H__
#define __QLUACAPI_H__

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

struct qactor_t;

/*
 * register all C APIs to Lua
 * */
void qluac_register(lua_State *state, struct qactor_t *);

/*
 *  Lua C API:
 *
 *  1) c_spawn: spawn an actor
 *               [IN]mod, fun, args table, [OUT]actor id
 *
 *
 *
 */
#endif  /* __QLUACAPI_H__ */
