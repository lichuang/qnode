#ifndef __LDB_H__
#define __LDB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <lua.h>
#include <lualib.h>                                                       
#include <lauxlib.h> 
#include "ldb_file.h"

typedef struct ldb_breakpoint_t {
  unsigned int  available:1;
  char         *file;
  char         *func;
  const char   *type;
  int           line;
  unsigned int  active:1;
  int           index;
  int           hit;
} ldb_breakpoint_t;

#define MAX_FILE_BUCKET 20
#define MAX_BREAKPOINT  60

struct ldb_t {
  lua_State        *state;

  int               call_depth;
  unsigned int      step:1;

  ldb_file_t       *files[MAX_FILE_BUCKET];

  int               bknum;
  ldb_breakpoint_t  bkpoints[MAX_BREAKPOINT];
};

ldb_t*  ldb_new(lua_State *state);
void    ldb_free(ldb_t *ldb);

void    ldb_break(lua_State *state);

#ifdef __cplusplus
}
#endif

#endif  /* __LDB_H__ */
