/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QSTRING_H__
#define __QSTRING_H__

#include <stdarg.h>
#include <stdio.h>
#include "qcore.h"

typedef struct qstring_t {
  qmem_pool_t *pool;
  size_t      len;
  size_t      size;
  char        *data;
} qstring_t;

#define qstring(str, pool)      { (pool), sizeof(str) - 1, sizeof(str) - 1, (str) }
#define qstring_null(pool)      { (pool), 0, 0, NULL }
#define qstring_set(str, text, p)                            \
      (str)->pool = (p); (str)->len = sizeof(text) - 1; (str)->size = (str)->size; (str)->data = text
#define qstring_null_set(str, p)   (str)->pool = (p); (str)->len = (str)->size = 0; (str)->data = NULL

/*
qstring_t*  qstring_new();
*/
void        qstring_destroy(qstring_t *string);
int         qstring_assign(qstring_t *string, const char *str);
int         qstring_append(qstring_t *string, const char *str);
int         qstring_format(qstring_t *string, char *fmt, ...);

#endif  /* __QSTRING_H__ */
