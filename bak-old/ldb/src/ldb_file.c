#include <stdlib.h>
#include <string.h>
#include "ldb.h"
#include "ldb_file.h"
#include "ldb_util.h"

static inline int   hashstring(const char *str);
static ldb_file_t*  do_load_file(const char *name);

#define LINE_NUM 100

static inline int 
hashstring(const char *str) {
  unsigned int hash = 5381;
  const char *s; 

  if (str == NULL) {
    return 0;
  }

  for (s = str; *s; s++) { 
    hash = ((hash << 5) + hash) + *s; 
  }
  hash &= 0x7FFFFFFF;

  return hash;
}

ldb_file_t*
ldb_file_load(ldb_t *ldb, const char *name) {
  int         hash;
  ldb_file_t *file;

  hash = hashstring(name) % MAX_FILE_BUCKET;
  file = ldb->files[hash];
  while (file) {
    if (strcmp(file->name, name)) {
      file = file->next;
      continue;
    }

    return file;
  }

  file = do_load_file(name);
  file->next = ldb->files[hash];
  ldb->files[hash] = file;

  return file;
}

static ldb_file_t*
do_load_file(const char *name) {
  ldb_file_t *file;
  char data[2048];  /* assume max chars per line == 2048 */
  FILE *f;
  int line, i;
  char *p;

  f = NULL;
  file = NULL;
  f = fopen(name, "r");
  if (f == NULL) {
    ldb_output("open file %s error\n");
    goto error;
  }

  file = (ldb_file_t*)malloc(sizeof(ldb_file_t));
  if (file == NULL) {
    goto error;
  }
  file->next  = NULL;
  file->name  = NULL;
  file->lines = NULL;
  file->alloc = LINE_NUM;
  file->line  = 0;

  file->name = strdup(name);
  if (file->name == NULL) {
    goto error;
  }
  file->lines = (char **)malloc(sizeof(char*) * LINE_NUM);
  if (file->lines == NULL) {
    goto error;
  }
  for (i = 0; i < LINE_NUM; ++i) {
    file->lines[i] = NULL;
  }
  line = 0;
  p = NULL;
  while ((p = fgets(data, sizeof(data), f)) != NULL) {
    if (line > file->alloc) {
      file->alloc += LINE_NUM;
      file->lines = (char **)realloc(file->lines,
                                     sizeof(char*) * file->alloc);
      if (file->lines == NULL) {
        goto error;
      }
      for (i = line; i < file->alloc; ++i) {
        file->lines[i] = NULL;
      }
    }

    file->lines[line] = strdup(p);
    if (file->lines[line] == NULL) {
      goto error;
    }
    ++line;
  }
  file->line = line;
  fclose(f);

  return file;

error:
  if (file) {
    if (file->name) {
      free(file->name);
    }
    if (file->lines) {
      for (i = 0; i < line; ++i) {
        free(file->lines[i]);
      }
      free(file->lines);
    }
    free(file);
  }
  if (f) {
    fclose(f);
  }
  return NULL;
}

void
ldb_file_free(ldb_file_t *file) {
  int i;

  for (i = 0; i < file->line; ++i) {
    free(file->lines[i]);
  }
  free(file->name);
  free(file->lines);
  free(file);
}
