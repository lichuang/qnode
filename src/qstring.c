/*
 * See Copyright Notice in qnode.h
 */

#include <string.h>
#include "qassert.h"
#include "qstring.h"
#include "qlog.h"
#include "qmalloc.h"

void qstring_init(qstring_t *string) {
  string->len = string->size = 0;
  string->data = NULL;
}

void qstring_destroy(qstring_t *string) {
  qassert(string);
  qassert(string->data);
  qfree(string->data);
}

static int string_reserve(qstring_t *string, size_t len, int need_copy) {
  size_t new_len = len + string->len;
  if (string->size < new_len) {
    char *data = qalloc_array(char, new_len * sizeof(char));
    if (data == NULL) {
      return -1;
    }
    if (string->len > 0) {
      if (need_copy) {
        strcpy(data, string->data);
      }
      if (string->data) {
        qfree(string->data);
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
  size_t len = strlen(str) + 1;
  if (string_reserve(string, len, 0) < 0) {
    return -1;
  }
  strcpy(string->data, str);
  string->len = len - 1;
  return 0;
}

int qstring_append(qstring_t *string, const char *str) {
  qassert(string);
  qassert(str);
  size_t len = strlen(str) + 1;
  if (string_reserve(string, len + string->len, 1) < 0) {
    return -1;
  }
  strcpy(string->data + string->len, str);
  string->len += len - 1;
  return 0;
}
