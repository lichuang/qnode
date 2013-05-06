/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QDICT_H__
#define __QDICT_H__

#include "qcore.h"
#include "qlist.h"
#include "qstring.h"

enum {
  QDICT_VAL_NUMBER    = 0,
  QDICT_VAL_STRING    = 1,
};

typedef struct qdict_entry_t {
  unsigned int    hash;

  qstring_t       key;

  union {
    int           num;
    qstring_t     str;
  } value;

  unsigned int    type:1;
  qlist_t         entry;
} qdict_entry_t;

typedef struct qdict_t {
  int             hashsize;
  unsigned int    num;
  qlist_t        *buckets[];
} qdict_t;

typedef struct qdict_iter_t {
  int             hash;
  qdict_t        *dict;
  qdict_entry_t  *entry;
} qdict_iter_t;

qdict_t*        qdict_new(int hashsize);
void            qdict_destroy(qdict_t *dict);
int             qdict_add(qdict_t *dict, qkey_t *key, qval_t *val);
int             qdict_replace(qdict_t *dict, qkey_t *key, qval_t *val);
qdict_entry_t*  qdict_get(qdict_t *dict, const char *key);

qdict_iter_t*   qdict_iterator(qdict_t *dict);
void            qdict_iterator_destroy(qdict_iter_t *iter);
qdict_entry_t*  qdict_next(qdict_iter_t *iter);

#endif  /* __QDICT_H__ */
