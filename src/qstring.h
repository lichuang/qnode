/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QSTRING_H__
#define __QSTRING_H__

#include <stdarg.h>
#include <stdio.h>

typedef char* qstring_t;

qstring_t   qstring_new(const char* data);
qstring_t   qstring_assign(qstring_t str, const char* data);
qstring_t   qstring_append(qstring_t string, const char *str);
int         qstring_equal(qstring_t str1, qstring_t str2);
int         qstring_equal_raw(qstring_t str1, const char* str2);
void        qstring_destroy(qstring_t string);

#endif  /* __QSTRING_H__ */
