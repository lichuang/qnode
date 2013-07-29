#ifndef __LDB_FILE_H__
#define __LDB_FILE_H__

#include "ldb_core.h"

struct ldb_file_t {
  char       *name;
  char      **lines;
  int         alloc;
  int         line;
  ldb_file_t *next;
};

ldb_file_t* ldb_file_load(ldb_t *ldb, const char *name);
void        ldb_file_free(ldb_file_t *file);

#endif  /* __LDB_FILE_H__ */
