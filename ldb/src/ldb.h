#ifndef __LDB_H__
#define __LDB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <lua.h>
#include <lualib.h>                                                       
#include <lauxlib.h> 

typedef struct ldb_breakpoint_t {
  const char   *file;
  int           line;
  unsigned int  active:1;
} ldb_breakpoint_t;

typedef struct ldb_t {
  lua_State    *state;

  int           call_depth;
  unsigned int  step:1;
} ldb_t;

ldb_t*  ldb_new();
void    ldb_destroy(ldb_t *ldb);

void    ldb_attach(ldb_t *ldb, lua_State *state);

void    ldb_step_in(lua_State *state, int step);

#ifdef __cplusplus
}
#endif

#endif  /* __LDB_H__ */
