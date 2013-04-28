/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QAPI_H__
#define __QAPI_H__

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "qcore.h"

/*
 * regist all C APIs to Lua
 * */
void qapi_register(lua_State *state, qactor_t *actor);

/*
 *  Lua C API:
 *
 *  1) actor 
 *
 *  qnode_spawn: spawn an actor
 *      [IN]mod, fun, args table
 *      [OUT]actor id
 *
 *  qnode_send: send message to an actor
 *      [IN]id, args table
 *      [OUT]NONE
 *
 *  qnode_controll: attach a descriptor to a node
 *      [IN]descriptor
 *      [OUT]NONE
 *
 *  2) tcp
 *
 *  qnode_tcp_listen: 
 *      [IN]port, listen params table
 *      [OUT]descriptor or nil
 *      [ERROR]nil, error message
 *
 *  qnode_tcp_accept: 
 *      [IN]descriptor, [, timeout]
 *      [OUT]descriptor or nil(timeout)
 *      [ERROR]nil, error message
 *
 *  qnode_tcp_recv: 
 *      [IN]socket [, timeout]
 *      [OUT]buffer
 *      [ERROR]nil, error message
 *
 *  qnode_tcp_send: 
 *      [IN]socket [, timeout]
 *      [OUT]0
 *      [ERROR]nil, error message
 *
 *  3) buffer
 *  qnode_buffer_find: 
 *      [IN]buffer, ch
 *      [OUT]pos(-1 if not find)
 *
 */

#endif  /* __QAPI_H__ */
