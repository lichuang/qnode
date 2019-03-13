/*
 * See Copyright Notice in qnode.h
 */

#include "qalloc.h"
#include "qassert.h"
#include "qdict.h"
#include "qlog.h"

static inline       int hashstring(const char *str);
static              int mainposition(qdict_t *dict, const char *key);
static qdict_node_t*    find_strkey(qdict_t *dict, const char *key,
                                    int *idx);
static qvalue_t*        set_strkey(qdict_t *dict, const char *key,
                                   qvalue_t *value);
static qdict_node_t*    find_numkey(qdict_t *dict, qnumber_t key,
                                    int *idx);
static qvalue_t*        set_numkey(qdict_t *dict, qnumber_t key,
                                   qvalue_t *value);

qdict_t*
qdict_new(int hashsize) {
  int       i;
  qdict_t  *dict;

  dict = qcalloc(sizeof(qdict_t) + hashsize * sizeof(qlist_t));
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

void
qdict_free(qdict_t *dict) {
  int           i;
  qlist_t      *list;
  qlist_t      *pos, *next;
  qdict_node_t *node;

  for (i = 0; i < dict->hashsize; ++i) {
    list = &(dict->buckets[i]);
    pos = list->next;
    while (pos != list) {
      node = qlist_entry(pos, qdict_node_t, entry);
      next = pos->next;
      qvalue_free(&(node->key));
      qvalue_free(&(node->value));
      qfree(node);
      pos  = next;
    }
  }
  qfree(dict);
}

static inline int
hashstring(const char *str) {
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

static int
mainposition(qdict_t *dict, const char *key) {
  return hashstring(key) % dict->hashsize;
}

static qdict_node_t*
find_strkey(qdict_t *dict, const char *key, int *idx) {
  int             hash;
  qlist_t        *list, *pos;
  qdict_node_t   *node;

  hash = mainposition(dict, key);
  if (idx != NULL) {
    *idx = hash;
  }
  list = &(dict->buckets[hash]);
  qlist_for_each(pos, list) {
    node = qlist_entry(pos, qdict_node_t, entry);
    if (!qvalue_isstring(&(node->key))) {
      continue;
    }
    if (qstring_compare(node->key.data.str, key, strlen(key)) == 0) {
      return node;
    }
  }

  return NULL;
}

static qvalue_t*
set_strkey(qdict_t *dict, const char *key, qvalue_t *value) {
  int           idx;
  qdict_node_t *node;

  node = find_strkey(dict, key, &idx);
  
  if (node) {
    qvalue_clone(&(node->value), value);
    return &(node->value);
  }

  node = qcalloc(sizeof(qdict_node_t));
  if (node == NULL) {
    return NULL;
  }
  memset(&node->value, 0, sizeof(qvalue_t));
  node->hash  = idx;
  qvalue_newstr(&(node->key), key);
  if (node->key.data.str == NULL) {
    qfree(node);
    return NULL;
  }
  qvalue_clone(&(node->value), value);
  qlist_entry_init(&(node->entry));
  qlist_add_tail(&(node->entry), &(dict->buckets[idx]));

  return &(node->value);
}

static qdict_node_t*
find_numkey(qdict_t *dict, qnumber_t key, int *idx) {
  int             hash;
  qlist_t        *list, *pos;
  qdict_node_t   *node;

  hash = key % dict->hashsize;
  if (idx != NULL) {
    *idx = hash;
  }
  list = &(dict->buckets[hash]);
  qlist_for_each(pos, list) {
    node = qlist_entry(pos, qdict_node_t, entry);
    if (!qvalue_isnumber(&(node->key))) {
      continue;
    }
    if (node->key.data.num == key) {
      return node;
    }
  }

  return NULL;
}

static qvalue_t*
set_numkey(qdict_t *dict, qnumber_t key, qvalue_t *value) {
  int           idx;
  qdict_node_t *node;

  node = find_numkey(dict, key, &idx);
  
  if (node) {
    qvalue_clone(&(node->value), value);
    return &(node->value);
  }

  node = qcalloc(sizeof(qdict_node_t));
  if (node == NULL) {
    return NULL;
  }
  memset(&node->value, 0, sizeof(qvalue_t));
  node->hash  = idx;
  qvalue_newnum(&(node->key), key);
  qvalue_clone(&(node->value), value);
  qlist_entry_init(&(node->entry));
  qlist_add_tail(&(node->entry), &(dict->buckets[idx]));

  return &(node->value);
}

qvalue_t*
qdict_set_strnum(qdict_t *dict, const char *key, qnumber_t num) {
  qvalue_t value = qvalue_number(num);

  return set_strkey(dict, key, &value);
}

qvalue_t*
qdict_set_strstr(qdict_t *dict, const char *key, const char* str) {
  qvalue_t value = qvalue_string(str);

  return set_strkey(dict, key, &value);
}

qvalue_t*
qdict_set_strdata(qdict_t *dict, const char *key, void* data) {
  qvalue_t value = qvalue_data(data, NULL);

  return set_strkey(dict, key, &value);
}

qvalue_t*
qdict_set_numdata(qdict_t *dict, qnumber_t key,
                  void* data, qvalue_free_pt *free) {
  qvalue_t value = qvalue_data(data, free);

  return set_numkey(dict, key, &value);
}

void
qdict_del_num(qdict_t *dict, qnumber_t key) {
  int           idx;
  qdict_node_t *node;

  node = find_numkey(dict, key, &idx);
  if (!node) {
    return;
  }

  qlist_del(&(node->entry));
  qvalue_free(&(node->key));
  qvalue_free(&(node->value));
  qfree(node);
}

qvalue_t*
qdict_get_str(qdict_t *dict, const char *key) {
  qdict_node_t *node;

  node = find_strkey(dict, key, NULL);
  if (node) {
    return &(node->value);
  }

  return NULL;
}

qvalue_t*
qdict_get_num(qdict_t *dict, qnumber_t key) {
  qdict_node_t *node;

  node = find_numkey(dict, key, NULL);
  if (node) {
    return &(node->value);
  }

  return NULL;
}

void
qdict_iterator_init(qdict_t *dict, qdict_iter_t *iter) {
  iter->dict  = dict;
  iter->hash  = 0;
  iter->node  = NULL;
}

qdict_node_t*
qdict_next(qdict_iter_t *iter) {
  qdict_t       *dict;
  qlist_t       *list, *pos;
  qdict_node_t  *node;

  dict = iter->dict;
  if (iter->node == NULL) {
    list = &(dict->buckets[iter->hash]);
    /* find the first non-empty hash list */
    while (qlist_empty(list)) {
      ++(iter->hash);
      if (iter->hash < dict->hashsize) {
        list = &(dict->buckets[iter->hash]);
      } else {
        return NULL;
      }
    }
    node = qlist_entry(list, qdict_node_t, entry);
  } else {
    pos = iter->node->entry.next;
    list = &(dict->buckets[iter->hash]);
    /* if reach the hash list end, get the next non-empty hash list */
    if (pos == list) {
      /* find the first non-empty hash list */
      do {
        ++(iter->hash);
        if (iter->hash < dict->hashsize) {
          list = &(dict->buckets[iter->hash]);
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
