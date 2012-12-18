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

#endif  /* __QLUACAPI_H__ */
