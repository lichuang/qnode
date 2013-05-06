/*
 * See Copyright Notice in qnode.h
 */

#include <string.h>
#include "qalloc.h"
#include "qassert.h"
#include "qstring.h"
#include "qlog.h"

qstring_t qstring_new(const char *data) {
  size_t          length;
  qstr_header_t  *header;

  length = (data == NULL) ? 0 : strlen(data);
  header  = qalloc(sizeof(qstr_header_t) + length + 1); 

  if (header = NULL) {
    return NULL;
  }

  if (data) {
    strncpy(header->data, data, length);
    header->data[length + 1] = '\0';
    header->free             = 0;
    header->length           = length;
  } else {
    header->len              = 0;
    header->free             = 0;
    str->data[0]             = '\0';
  }

  return &(header->data[0]);
}

void qstring_destroy(qstring_t string) {
  qstr_header_t *header;

  header = str_to_header(string);
  qfree(header);
}

static qstring_t string_reserve(qstring_t string, size_t len) {
  size_t          new_len;
  char           *data;
  qstr_header_t  *header;

  header = str_to_header(string);
  if (header->free > len) {
    return string;
  }

  new_len = header->len + len;
  header = qrealloc(header, new_len);
  if (header == NULL) {
    return NULL;
  }
  header->free = new_len - header->len;

  return string;
}

/*
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
*/

qstring_t qstring_append(qstring_t string, const char *str) {
  size_t len;
  qstr_header_t *header;

  qassert(string);
  qassert(str);

  len = strlen(str);
  if (string_reserve(string, len + 1) == NULL) {
    return NULL;
  }

  header = str_to_header(string);
  strncpy(string + header->len, str, len);
  string[header->len + 1] = '\0';
  header->len += len;
  header->free -= len;

  return string;
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
