/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QDICT_H__
#define __QDICT_H__

#include "qcore.h"
#include "qlist.h"
#include "qstring.h"
#include "qvalue.h"

typedef struct qdict_node_t {
  unsigned int    hash;
  qstring_t       key;
  qvalue_t        value;
  qlist_t         entry;
} qdict_node_t;

typedef struct qdict_t {
  int             hashsize;
  unsigned int    num;
  qlist_t        *buckets[];
} qdict_t;

typedef struct qdict_iter_t {
  int             hash;
  qdict_t        *dict;
  qdict_node_t   *node;
} qdict_iter_t;

qdict_t*        qdict_new(int hashsize);
void            qdict_destroy(qdict_t *dict);

qvalue_t*       qdict_setnum(qdict_t *dict, const char *key, qnumber_t num);
qvalue_t*       qdict_setstr(qdict_t *dict, const char *key, const char* str);
qvalue_t*       qdict_get(qdict_t *dict, const char *key);

#define         qdict_iter(dict)  { .dict = (dict), .hash = 0, .node = NULL }

qdict_node_t*   qdict_next(qdict_iter_t *iter);

#endif  /* __QDICT_H__ */
