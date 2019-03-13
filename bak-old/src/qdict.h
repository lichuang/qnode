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
  qvalue_t        key;
  qvalue_t        value;
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

qdict_t*  qdict_new(int hashsize);
void      qdict_free(qdict_t *dict);

qvalue_t* qdict_set_strnum(qdict_t *dict, const char *key, qnumber_t num);
qvalue_t* qdict_set_strstr(qdict_t *dict, const char *key, const char* str);
qvalue_t* qdict_set_strdata(qdict_t *dict, const char *key, void* data);

qvalue_t* qdict_set_numdata(qdict_t *dict, qnumber_t key, void* data, qvalue_free_pt *free);

void      qdict_del_num(qdict_t *dict, qnumber_t key);

qvalue_t* qdict_get_str(qdict_t *dict, const char *key);
qvalue_t* qdict_get_num(qdict_t *dict, qnumber_t key);

#define   qdict_iter(dict)  { .dict = (dict), .hash = 0, .node = NULL }

qdict_node_t*   qdict_next(qdict_iter_t *iter);

#endif /* __QDICT_H__ */
