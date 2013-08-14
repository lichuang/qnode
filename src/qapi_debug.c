/*
 * See Copyright Notice in qnode.h
 */

#include <lauxlib.h>
#include "qdefines.h"
#include "qlog.h"
#ifdef DEBUG
#include "ldb.h"
#endif

static int qlbreak(lua_State *state);

luaL_Reg debug_apis[] = {
  {"qlbreak",          qlbreak},
  {NULL, NULL},
};

static int
qlbreak(lua_State *state) {
#ifdef DEBUG  
  ldb_break(state);
#else
  UNUSED(state);
  qerror("debug API disabled, try define DEBUG and recompile");
#endif

  return 0;
}
