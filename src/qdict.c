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
  qlist_t  *list;

  dict = qcalloc(sizeof(qdict_t) + hashsize * sizeof(qlist_t *));
  if (dict == NULL) {
    return NULL;
  }
  dict->hashsize = hashsize;
  dict->num = 0;
  for (i = 0; i < hashsize; ++i) {
    qlist_entry_init(&(dict->buckets[i]));
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
      qvalue_destroy(node->value);
      qfree(node);
      pos  = next;
    }
  }
  qfree(dict);
}

static inline int hashstring(const char *str, size_t len) {
  unsigned int hash = 5381;

  while (len--) {
    hash = ((hash << 5) + hash) + (*str++); /* hash * 33 + c */
  }
  return hash;
}

static int mainposition(qdict_t *dict, qstring_t key) {
  qstr_header_t *header;

  header = str_to_header(key);
  return hashstring(key, header->len) % dict->hashsize;
}

static qdict_node_t* find(qdict_t *dict, qstring_t *key, int *idx) {
  int             idx;
  qlist_t        *list, *pos;
  qdict_node_t  *node;

  hash = mainposition(dict, key);
  if (idx != NULL) {
    *idx = hash;
  }
  list = dict->buckets[hash];
  qlist_for_each(pos, list) {
    node = qlist_entry(pos, qdict_node_t, entry);
    if (qstring_equal(node->key, key) == 0) {
      return node;
    }
  }

  return NULL;
}

qdict_node_t*  qdict_set(qdict_t *dict,
                         const char *key, qvalue_t *value) {
  int idx;
  qdict_node_t *node;
  qstring_t str;

  str = qstring_new(key);
  if (str == NULL) {
    return NULL;
  }

  node = find(dict, str, &idx);
  
  if (node) {
    qvalue_destroy(node->value);
    node->value = value;
    qstring_destroy(str);

    return entry;
  }

  node = qalloc(sizeof(qdict_node_t));
  if (node == NULL) {
    qstring_destroy(str);
    return NULL;
  }
  node->hash  = idx;
  node->key   = str;
  node->value = value;
  qlist_add(&(node->entry), &(dict->buckets[idx]));

  return node;
}

qdict_node_t*  qdict_get(qdict_t *dict, const char *key) {
  qdict_node_t *node;
  qstring_t str;

  str = qstring_new(key);
  if (str == NULL) {
    return NULL;
  }
  node = find(dict, str, NULL);
  qstring_destroy(str);

  return node;
}

qdict_iter_t* qdict_iterator(qdict_t *dict) {
  qdict_iter_t *iter;

  iter = qalloc(sizeof(qdict_iter_t));
  iter->dict  = dict;
  iter->hash  = 0;
  iter->entry = NULL;
  return iter;
}

void qdict_iterator_destroy(qdict_iter_t *iter) {
  qfree(iter);
}

qdict_node_t* qdict_next(qdict_iter_t *iter) {
  qdict_t       *dict;
  qlist_t       *list, *pos;
  qdict_node_t *node;

  dict = iter->dict;
  if (iter->node == NULL) {
    list = dict->buckets[iter->hash];
    /* find the first non-empty hash list */
    while (qlist_empty(list)) {
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
