#include <stdio.h>
#include "ldb.h"

ldb_t *ldb;

static int
c_break(lua_State *state) {
  ldb_break(state);
  return 0;
}

int main() {
  int i;
  const char *file = "my.lua";

  lua_State *L = lua_open();
  luaL_openlibs(L);

  lua_register(L, "c_break", c_break);

  ldb = ldb_new(L);
  luaL_dofile(L, file);

  ldb_free(ldb);
  lua_close(L);

  return 0;
}
