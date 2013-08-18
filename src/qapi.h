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
void qapi_register(lua_State *state);

/*
 * Lua C API:
 *   1) Buffer
 *
 *     qlbuffer_find: find a str from pos
 *     [IN]buffer, pos, str
 *     [OUT]pos
 *
 *     qlbuffer_rlen: return read length of the buffer
 *     [IN]buffer
 *     [OUT]buffer read len
 *
 *     qlbuffer_wlen: return write length of the buffer
 *     [IN]buffer
 *     [OUT]buffer write len
 *
 *     qlbuffer_write_string: write a string into the buffer
 *     [IN]buffer, string
 *     [OUT]OK
 *
 *     qlbuffer_write_char: write a char into the buffer
 *     [IN]buffer, string(only one char)
 *     [OUT]OK
 *
 *     qlbuffer_reset: reset the buffer
 *     [IN]buffer
 *     [OUT]OK
 *
 *     qlbuffer_set: set the buffer str at pos
 *     [IN]buffer, pos, str
 *     [OUT]OK
 *
 *     qlbuffer_get: get the buffer length str from pos
 *     [IN]buffer, pos, length
 *     [OUT]string
 */

#endif  /* __QAPI_H__ */
