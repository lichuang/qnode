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
 * regist all C APIs to Lua
 * */
void qluac_register(lua_State *state, struct qactor_t *);

/*
 *  Lua C API:
 *
 *  1) actor 
 *
 *  spawn: spawn an actor
 *      [IN]mod, fun, args table
 *      [OUT]actor id
 *
 *  send: send message to an actor
 *      [IN]id, args table
 *      [OUT]none
 *
 *  2) timer
 *
 */
#endif  /* __QLUACAPI_H__ */
