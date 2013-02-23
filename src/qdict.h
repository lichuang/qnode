/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QDICT_H__
#define __QDICT_H__

#include "qlist.h"
#include "qstring.h"

enum {
  QDICT_KEY_NUMBER = 0,
  QDICT_KEY_STRING = 1,
};

typedef struct qkey_t {
  union {
    int num;
    const char *str;
  } data;

  unsigned short type;
} qkey_t;

#define QKEY_NUMBER(key, n) do { (key).data.num = (n); (key).type = QDICT_KEY_NUMBER; } while(0)
#define QKEY_STRING(key, s) do { (key).data.str = (s); (key).type = QDICT_KEY_STRING; } while(0)

enum {
  QDICT_VAL_NUMBER = 0,
  QDICT_VAL_STRING = 1,
  QDICT_VAL_USERDATA = 2,
};

typedef struct qdict_key_t {
  union {
    int num;
    qstring_t *str;
  } data;

  unsigned short type;
} qdict_key_t;

typedef struct qdict_val_t {
  union {
    int num;
    qstring_t *str;
    void *ptr;
  } data;
  unsigned short type;
} qdict_val_t;

#define QVAL_NUMBER(val, n) do { (val).data.num = (n); (val).type = QDICT_VAL_NUMBER; } while(0)
#define QVAL_STRING(val, s) do { (val).data.str = (s); (val).type = QDICT_VAL_STRING; } while(0)

typedef struct qdict_entry_t {
  unsigned int hash;
  qlist_t entry;
  qdict_key_t key;
  qdict_val_t val;
} qdict_entry_t;

typedef struct qdict_t {
  qlist_t **buckets;
  unsigned int hashsize;
  unsigned int num;
} qdict_t;

typedef struct qdict_iter_t {
  qdict_t *dict;
  unsigned int hash;
  qdict_entry_t *entry;
} qdict_iter_t;

qdict_t*        qdict_new(unsigned int hashsize);
void            qdict_destroy(qdict_t *dict);
int             qdict_add(qdict_t *dict, qkey_t *key, qdict_val_t *val);
int             qdict_replace(qdict_t *dict, qkey_t *key, qdict_val_t *val);
qdict_val_t*    qdict_get(qdict_t *dict, qkey_t *key);
qdict_iter_t*   qdict_iterator(qdict_t *dict);
qdict_entry_t*  qdict_next(qdict_iter_t *iter);

#endif  /* __QDICT_H__ */
