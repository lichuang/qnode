/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QSTRING_H__
#define __QSTRING_H__

#include <stdarg.h>
#include <stdio.h>
#include "qcore.h"

#define str_to_header(str) (qstr_header_t*)((str) - sizeof(qstr_header_t))

typedef char* qstring_t;

typedef struct qstr_header_t {
  size_t      len;
  size_t      free;
  char        data[];
} qstr_header_t;

/*
#define qstring(str)      { sizeof(str) - 1, sizeof(str) - 1, (str) }
#define qstring_null()    { 0, 0, NULL }
#define qstring_set(str, text)                            \
      (str)->len = sizeof(text) - 1; (str)->size = (str)->size; (str)->data = text
#define qstring_null_set(str)   (str)->len = (str)->size = 0; (str)->data = NULL
*/

qstring_t   qstring_new(const char* data);
qstring_t   qstring_assign(qstring_t str, const char* data);
void        qstring_destroy(qstring_t string);
qstring_t   qstring_append(qstring_t string, const char *str);
int         qstring_equal(qstring_t str1, qstring_t str2);
int         qstring_equal_raw(qstring_t str1, const char* str2);

#endif  /* __QSTRING_H__ */
