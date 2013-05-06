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

typedef struct qdict_node_t {
  unsigned int    hash;

  qstring_t       key;
  qvalue_t       *value;

  qlist_t         entry;
} qdict_node_t;

typedef struct qdict_t {
  int             hashsize;
  unsigned int    num;
  qlist_t         buckets[];
} qdict_t;

typedef struct qdict_iter_t {
  int             hash;
  qdict_t        *dict;
  qdict_node_t   *node;
} qdict_iter_t;

#define qdict_entry_key(entry)   ((entry)->key)
#define qdict_entry_value(entry) ((entry)->value)

qdict_t*        qdict_new(int hashsize);
void            qdict_destroy(qdict_t *dict);

qdict_node_t*   qdict_set(qdict_t *dict,
                          const char *key, qvalue_t *value);
qdict_node_t*   qdict_get(qdict_t *dict, const char *key);

qdict_iter_t*   qdict_iterator(qdict_t *dict);
void            qdict_iterator_destroy(qdict_iter_t *iter);
qdict_node_t*   qdict_next(qdict_iter_t *iter);

#endif  /* __QDICT_H__ */
