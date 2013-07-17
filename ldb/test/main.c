#include "ldb.h"

ldb_t *ldb;

static int
c_break(lua_State *state) {
  ldb_attach(ldb, state);
  ldb_step_in(state, 1);
  return 0;
}

int main() {
  int i;
  const char *file = "my.lua";

  lua_State *L = lua_open();
  luaL_openlibs(L);

  lua_register(L, "c_break", c_break);

  ldb = ldb_new();
  luaL_dofile(L, file);

  ldb_destroy(ldb);

  return 0;
}
