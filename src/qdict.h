/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QDICT_H__
#define __QDICT_H__

#include "qcore.h"
#include "qlist.h"
#include "qstring.h"

enum {
  QDICT_KEY_NUMBER    = 0,
  QDICT_KEY_STRING    = 1,
};

enum {
  QDICT_VAL_NUMBER    = 0,
  QDICT_VAL_STRING    = 1,
  QDICT_VAL_USERDATA  = 2,
};

typedef struct qkey_t {
  union {
    int           num;
    qstring_t     str;
  } data;

  unsigned short  type;
} qkey_t;

#define QKEY_NUMBER(key, n) do {        \
  (key).data.num = (n);                 \
  (key).type = QDICT_KEY_NUMBER;        \
} while(0)

#define QKEY_STRING(key, s, p) do {       \
  qstring_set(&((key).data.str), s, (p)); \
  (key).type = QDICT_KEY_STRING;          \
} while(0)

typedef struct qval_t {
  union {
    int           num;
    qstring_t     str;
    void          *ptr;
  } data;
  unsigned short  type;
} qval_t;

#define QVAL_NUMBER(val, n) do { (val).data.num = (n); (val).type = QDICT_VAL_NUMBER; } while(0)
#define QVAL_STRING(val, s) do { (val).data.str = (s); (val).type = QDICT_VAL_STRING; } while(0)

typedef struct qdict_entry_t {
  unsigned int  hash;
  qlist_t       entry;
  qkey_t        key;
  qval_t        val;
} qdict_entry_t;

typedef struct qdict_t {
  qlist_t       **buckets;
  int           hashsize;
  unsigned int  num;
  qmem_pool_t   *pool;
} qdict_t;

typedef struct qdict_iter_t {
  qdict_t       *dict;
  int           hash;
  qdict_entry_t *entry;
} qdict_iter_t;

qdict_t*        qdict_new(qmem_pool_t *pool, int hashsize);
void            qdict_destroy(qdict_t *dict);
int             qdict_add(qdict_t *dict, qkey_t *key, qval_t *val);
int             qdict_replace(qdict_t *dict, qkey_t *key, qval_t *val);
qval_t*         qdict_get(qdict_t *dict, qkey_t *key);
qdict_iter_t*   qdict_iterator(qdict_t *dict);
void            qdict_iterator_destroy(qdict_iter_t *iter);
qdict_entry_t*  qdict_next(qdict_iter_t *iter);

#endif  /* __QDICT_H__ */
