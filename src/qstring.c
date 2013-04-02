/*
 * See Copyright Notice in qnode.h
 */

#include <string.h>
#include "qassert.h"
#include "qmempool.h"
#include "qstring.h"
#include "qlog.h"

/*
qstring_t* qstring_new() {
  qstring_t *str  = qalloc_type(qstring_t); 
  str->len = str->size = 0;
  str->data = NULL;
  return str;
}
*/

void qstring_destroy(qstring_t *string) {
  qassert(string);
  qassert(string->data);
  if (string->pool) {
    qfree(string->pool, string->data, string->size);
  }
}

static int string_reserve(qstring_t *string, size_t len, int need_copy) {
  size_t   new_len;
  char    *data;

  new_len = len + string->len;
  if (string->size < new_len) {
    data = qalloc(string->pool, new_len * sizeof(char));
    if (data == NULL) {
      return -1;
    }
    if (string->len > 0) {
      if (need_copy) {
        strncpy(data, string->data, string->len);
      }
      if (string->data) {
        qfree(string->pool, string->data, string->size);
      }
    }
    string->data = data;
    string->size = new_len;
  }

  return 1;
}

int qstring_assign(qstring_t *string, const char *str) {
  qassert(string);
  qassert(str);
  size_t len;

  len = strlen(str) + 1;
  if (string_reserve(string, len, 0) < 0) {
    return -1;
  }
  strncpy(string->data, str, len);
  string->len = len - 1;

  return 0;
}

int qstring_append(qstring_t *string, const char *str) {
  qassert(string);
  qassert(str);
  size_t len;

  len = strlen(str) + 1;
  if (string_reserve(string, len, 1) < 0) {
    return -1;
  }
  strncpy(string->data + string->len, str, len);
  string->len += len - 1;

  return 0;
}

int qstring_format(qstring_t *string, char *fmt, ...) {
  va_list args;
  int     ret;

  if (string_reserve(string, 200, 0) < 0) {
    return -1;
  }
  va_start(args, fmt);
  ret = vsnprintf(string->data, 200, fmt, args);
  va_end(args);

  return ret;
}
