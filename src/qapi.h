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
 * Lua C API
 *
 * NOTE:
 *   Most qnode lua C api has follow form:
 *   ret, error = qlapi()
 *   if ret then
 *     do_func()
 *   else
 *     log(error)
 *   end
 *   if the api has no fault, it will return a not-nil value,
 *   else ret will be nil and the error msg will be returned in error,
 *   users can log the error msg to see what happened.
 *
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
 *
 *  2) debug
 *     qlbreak: break and into debug model,users can use
 *     this to debug lua script.It only can be used if macro DEBUG set.
 *     [IN]no
 *     [OUT]no
 *
 *  3) log
 *     qllog: log a message
 *     [IN]format message string
 *     [OUT]no
 *
 *     qlerror: log error a message
 *     [IN]format message string
 *     [OUT]no
 *
 *  4) net
 *     qltcp_listen: create listen socket
 *     [IN]port, addr(default 0.0.0.0)
 *     [OUT]socket
 *
 *     qltcp_accept: accept a connection from listen socket
 *     [IN]listen socket
 *     [OUT]connection socket
 *
 *     qltcp_recv: recv data from connection socket
 *     [IN]connection socket
 *     [OUT]bytes received
 *
 *     qltcp_send: send data in in buffer from connection socket
 *     [IN]connection socket
 *     [OUT]bytes sent
 *
 *     qltcp_inbuf: connection socket in buffer, after get in buffer,
 *     users can use buffer API to read data in buffer.
 *     [IN]connection socket
 *     [OUT]in buffer
 *
 *     qltcp_outbuf: connection socket out buffer, after get out buffer,
 *     users can use buffer API to write data in buffer.
 *     [IN]connection socket
 *     [OUT]out buffer
 *
 *  5) node
 *
 *     qlnode_spawn: spawn a node to execute lua script
 *     [IN]mod, func, args table
 *     [OUT]node actor id
 *
 *     qlnode_send: send a message to node
 *     [IN]node actor id, args table
 *     [OUT]0
 *
 *     qlnode_recv: recv a message to node
 *     [IN]NONE
 *     [OUT]message
 *
 *     qlnode_attach: attach a socket to node,and it will be detach from old node
 *     [IN]socket
 *     [OUT]NONE
 *
 *     qlnode_self: return node actor id
 *     [IN]NONE
 *     [OUT]node actor id
 *
 *     qlnode_exit: a node exit
 *     [IN]NONE
 *     [OUT]NONE
 *
 *  6) string
 *
 *     qlstring_toul: use strtoull(base 10) for a lua string
 *     [IN]string
 *     [OUT]strtoull result
 */

#endif  /* __QAPI_H__ */
