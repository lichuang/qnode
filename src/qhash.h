/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QHASH_H__
#define __QHASH_H__

#include "qlist.h"

#define QHASH_KEY_NUMBER 1
#define QHASH_KEY_STRING 2

typedef struct qhash_key_t {
  union {
    unsigned int num;
    const char *str;
  } val;
  unsigned short type;
} qhash_key_t;

#define qhash_number_key(number) {{number}, QHASH_KEY_NUMBER}
#define qhash_string_key(string) {{string}, QHASH_KEY_STRING}

typedef struct qhash_entry_t {
  qlist_t entry;
  qhash_key_t *key;
  void *data;
} qhash_entry_t;

typedef struct qhash_t {
  int hsize;
  int num;
  qlist_t **buckets;
  qlist_t free;
} qhash_t;

qhash_t*        qhash_new(int hsize);
void            qhash_destroy(qhash_t *hash);

int             qhash_add(qhash_t *hash, qhash_key_t *key, void *data);
qhash_entry_t*  qhash_get(qhash_t *hash, qhash_key_t *key);
int             qhash_del(qhash_t *hash, qhash_key_t *key);

#endif  /* __QHASH_H__ */
