/*
 * See Copyright Notice in qnode.h
 */

#include <lauxlib.h>
#include "qdefines.h"
#include "qlog.h"
#ifdef DEBUG
#include "ldb.h"
#endif

static int qldebug(lua_State *state);

luaL_Reg debug_apis[] = {
  {"qldebug",          qldebug},
  {NULL, NULL},
};

static int
qldebug(lua_State *state) {
#ifdef DEBUG  
  ldb_step_in(state, 1);
#else
  UNUSED(state);
  qerror("debug API disabled, try define DEBUG and recompile");
#endif

  return 0;
}
