/*
 * See Copyright Notice in qnode.h
 */

#include "qalloc.h"
#include "qassert.h"
#include "qdict.h"
#include "qlog.h"

qdict_t* qdict_new(int hashsize) {
  int       i;
  qdict_t  *dict;

  dict = qcalloc(sizeof(qdict_t) + hashsize * sizeof(qlist_t));
  if (dict == NULL) {
    return NULL;
  }
  dict->hashsize = hashsize;
  dict->num = 0;
  for (i = 0; i < hashsize; ++i) {
    dict->buckets[i] = NULL;
  }

  return dict;
}

void qdict_destroy(qdict_t *dict) {
  int           i, hashsize;
  qlist_t      *list;
  qlist_t      *pos, *next;
  qdict_node_t *node;

  hashsize = dict->hashsize;
  for (i = 0; i < dict->hashsize; ++i) {
    list = dict->buckets[i];
    if (list == NULL) {
      continue;
    }
    for (pos = list->next; pos != list; ) {
      node = qlist_entry(pos, qdict_node_t, entry);
      next = pos->next;
      qstring_destroy(node->key);
      qvalue_destroy(&(node->value));
      qfree(node);
      pos  = next;
    }
  }
  qfree(dict);
}

static inline int hashstring(const char *str) {
  unsigned int hash = 5381;
  const char *s;

  if (str == NULL) {
    return 0;
  }

  for (s = str; *s; s++) { 
    hash = ((hash << 5) + hash) + *s;
  }
  hash &= 0x7FFFFFFF;

  return hash;
}

static int mainposition(qdict_t *dict, const char *key) {
  return hashstring(key) % dict->hashsize;
}

static qdict_node_t* find(qdict_t *dict, const char *key, int *idx) {
  int             hash;
  qlist_t        *list, *pos;
  qdict_node_t   *node;

  hash = mainposition(dict, key);
  if (idx != NULL) {
    *idx = hash;
  }
  list = dict->buckets[hash];
  qlist_for_each(pos, list) {
    node = qlist_entry(pos, qdict_node_t, entry);
    if (qstring_equal_raw(node->key, key) == 0) {
      return node;
    }
  }

  return NULL;
}

static qvalue_t* set(qdict_t *dict, const char *key, qvalue_t *value) {
  int           idx;
  qdict_node_t *node;

  node = find(dict, key, &idx);
  
  if (node) {
    qvalue_clone(&(node->value), value);

    return &(node->value);
  }

  node = qalloc(sizeof(qdict_node_t));
  if (node == NULL) {
    return NULL;
  }
  node->hash  = idx;
  node->key   = qstring_new(key);
  if (node->key == NULL) {
    return NULL;
  }
  qvalue_clone(&(node->value), value);
  if (dict->buckets[idx] == NULL) {
    dict->buckets[idx] = &(node->entry);
  } else {
    qlist_add(&(node->entry), dict->buckets[idx]);
  }

  return &(node->value);
}

qvalue_t* qdict_setnum(qdict_t *dict, const char *key, qnumber_t num) {
  qvalue_t value = qvalue_number(num);

  return set(dict, key, &value);
}

qvalue_t* qdict_setstr(qdict_t *dict, const char *key, const char* str) {
  qvalue_t value = qvalue_string(str);

  return set(dict, key, &value);
}

qvalue_t* qdict_get(qdict_t *dict, const char *key) {
  qdict_node_t *node;

  node = find(dict, key, NULL);
  if (node) {
    return &(node->value);
  }

  return NULL;
}

void qdict_iterator_init(qdict_t *dict, qdict_iter_t *iter) {
  iter->dict  = dict;
  iter->hash  = 0;
  iter->node  = NULL;
}

qdict_node_t* qdict_next(qdict_iter_t *iter) {
  qdict_t       *dict;
  qlist_t       *list, *pos;
  qdict_node_t  *node;

  dict = iter->dict;
  if (iter->node == NULL) {
    list = dict->buckets[iter->hash];
    /* find the first non-empty hash list */
    while (list == NULL) {
      ++(iter->hash);
      if (iter->hash < dict->hashsize) {
        list = dict->buckets[iter->hash];
      } else {
        return NULL;
      }
    }
    node = qlist_entry(list->next, qdict_node_t, entry);
  } else {
    pos = iter->node->entry.next;
    list = dict->buckets[iter->hash];
    /* if reach the hash list end, get the next non-empty hash list */
    if (pos == list) {
      /* find the first non-empty hash list */
      do {
        ++(iter->hash);
        if (iter->hash < dict->hashsize) {
          list = dict->buckets[iter->hash];
        } else {
          return NULL;
        }
      } while (qlist_empty(list));
      pos = list->next;
    }
    node = qlist_entry(pos, qdict_node_t, entry);
  }
  iter->node = node;

  return node;
}
