/*
 * See Copyright Notice in qnode.h
 */

#include <string.h>
#include "qalloc.h"
#include "qassert.h"
#include "qstring.h"
#include "qlog.h"

qstring_t qstring_new(const char *data) {
  size_t          len;
  qstr_header_t  *header;

  len = (data == NULL) ? 0 : strlen(data);
  header = qalloc(sizeof(qstr_header_t) + len + 1); 

  if (header == NULL) {
    return NULL;
  }

  if (data) {
    strncpy(header->data, data, len);
  }
  header->data[len] = '\0';
  header->free      = 0;
  header->len       = len;

  return &(header->data[0]);
}

void qstring_destroy(qstring_t string) {
  qstr_header_t *header;

  header = str_to_header(string);
  qfree(header);
}

static qstring_t string_reserve(qstr_header_t *header, size_t len) {
  size_t          new_len;

  if (header->free > len) {
    return header->data;
  }

  new_len = header->len + len;
  header = qrealloc(header, new_len);
  if (header == NULL) {
    return NULL;
  }
  header->free = new_len - header->len;

  return header->data;
}

qstring_t qstring_assign(qstring_t str, const char *data) {
  qstr_header_t *header;
  size_t len;

  if (data == NULL) {
    return str;
  }

  len = strlen(data);
  header = str_to_header(str);
  if (string_reserve(header, len + 1) == NULL) {
    return NULL;
  }
  strncpy(str, data, len);
  header->len = len;
  header->free -= len;
  header->data[len] = '\0';

  return str;
}

qstring_t qstring_append(qstring_t str, const char *data) {
  size_t len;
  qstr_header_t *header;

  len = strlen(data);
  header = str_to_header(str);
  if (string_reserve(header, len + 1) == NULL) {
    return NULL;
  }

  strncpy(str + header->len, data, len);
  header->len += len;
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
  size_t len;
  qstr_header_t *header;

  header = str_to_header(str1);
  len = strlen(str2);
  if (len != header->len) {
    return (header->len - len);
  }

  return (strcmp(str1, str2));
}
