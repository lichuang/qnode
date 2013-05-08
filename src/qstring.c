/*
 * See Copyright Notice in qnode.h
 */

#include <string.h>
#include "qalloc.h"
#include "qassert.h"
#include "qstring.h"
#include "qlog.h"

#define str_to_header(str) (void*)((str) - sizeof(qstr_header_t))

typedef struct qstr_header_t {
  size_t      len;
  size_t      free;
  char        data[];
} qstr_header_t;

qstring_t qstring_new(const char *data) {
  size_t          len;
  qstr_header_t  *header;

  len = (data == NULL) ? 0 : strlen(data) + 1;
  header = qalloc(sizeof(qstr_header_t) + len); 

  if (header == NULL) {
    return NULL;
  }

  if (data) {
    strncpy(header->data, data, len);
  }
  header->data[len - 1] = '\0';
  header->free          = 0;
  header->len           = len - 1;

  return header->data;
}

void qstring_destroy(qstring_t str) {
  qstr_header_t *header;

  header = str_to_header(str);
  qfree(header);
}

static qstring_t reserve(qstr_header_t *header, size_t len) {
  size_t new_len;

  if (header->free > len) {
    return header->data;
  }

  new_len = header->len + len;
  header  = qrealloc(header, new_len);
  if (header == NULL) {
    return NULL;
  }
  header->free = new_len - header->len;

  return header->data;
}

qstring_t qstring_assign(qstring_t str, const char *data) {
  size_t         len;
  qstr_header_t *header;

  if (data == NULL) {
    return str;
  }

  len    = strlen(data) + 1;
  header = str_to_header(str);
  if (reserve(header, len) == NULL) {
    return NULL;
  }
  strncpy(str, data, len);
  header->len = len;
  header->free -= len;
  header->data[len - 1] = '\0';

  return str;
}

qstring_t qstring_append(qstring_t str, const char *data) {
  size_t         len;
  qstr_header_t *header;

  len    = strlen(data) + 1;
  header = str_to_header(str);
  if (reserve(header, len) == NULL) {
    return NULL;
  }

  strncpy(str + header->len, data, len);
  header->len += len - 1;
  str[header->len] = '\0';
  header->free -= len;

  return str;
}

int qstring_equal(qstring_t str1, qstring_t str2) {
  qstr_header_t *header1, *header2;

  header1 = str_to_header(str1);
  header2 = str_to_header(str2);

  if (header1->len != header2->len) {
    return (header1->len - header2->len);
  }

  return (strcmp(str1, str2));
}

int qstring_equal_raw(qstring_t str1, const char* str2) {
  size_t         len;
  qstr_header_t *header;

  len    = strlen(str2);
  header = str_to_header(str1);
  if (len != header->len) {
    return (header->len - len);
  }

  return (strcmp(str1, str2));
}
