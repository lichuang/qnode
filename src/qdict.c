/*
 * See Copyright Notice in qnode.h
 */

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <string.h>
#include "qalloc.h"
#include "qassert.h"
#include "qdict.h"
#include "qlog.h"

qdict_t* qdict_new(int hashsize) {
  int       i;
  qdict_t  *dict;
  qlist_t  *list;

  dict = qalloc(sizeof(qdict_t));
  if (dict == NULL) {
    goto error;
  }
  dict->hashsize = hashsize;
  dict->num = 0;
  dict->buckets  = qcalloc(hashsize * sizeof(qlist_t*));
  if (dict->buckets == NULL) {
    goto error;
  }
  for (i = 0; i < hashsize; ++i) {
    list = qalloc(sizeof(qlist_t));
    if (list == NULL) {
      goto error;
    }
    qlist_entry_init(list);
    dict->buckets[i] = list;
  }
  return dict;

error:
  for (i = 0; i < hashsize; ++i) {
    list = dict->buckets[i];
    if (list != NULL) {
      qfree(list);
      continue;
    }
    break;
  }
  if (dict->buckets != NULL) {
    qfree(dict->buckets);
  }
  if (dict != NULL) {
    qfree(dict);
  }
  return NULL;
}

void qdict_destroy(qdict_t *dict) {
  int           i, hashsize;
  qlist_t      *list;
  qlist_t      *pos, *next;

  hashsize = dict->hashsize;
  for (i = 0; i < dict->hashsize; ++i) {
    list = dict->buckets[i];
    for (pos = list->next; pos != list; ) {
      qdict_entry_t *entry = qlist_entry(pos, qdict_entry_t, entry);
      next = pos->next;
      if (entry->key.type == QDICT_KEY_STRING) {
        qstring_destroy(&(entry->key.data.str));
      }
      if (entry->val.type == QDICT_VAL_STRING) {
        qstring_destroy(&(entry->val.data.str));
      }
      qfree(entry);
      pos  = next;
    }
    qfree(list);
  }
  qfree(dict->buckets);
  qfree(dict);
}

static int hashstring(const char *str, size_t len) {
  unsigned int hash = 5381;

  while (len--) {
    hash = ((hash << 5) + hash) + (*str++); /* hash * 33 + c */
  }
  return hash;
}

static int mainposition(qdict_t *dict, qkey_t *key) {
  if (key->type == QDICT_KEY_NUMBER) {
    return (key->data.num % dict->hashsize);
  }
  if (key->type == QDICT_KEY_STRING) {
    return hashstring(key->data.str.data, key->data.str.len) % dict->hashsize;
  }
  /* NEVER reach here */
  qerror("key type %d error", key->type);
  return -1;
}

static int compare(qkey_t *dict_key, qkey_t *key) {
  if (dict_key->type != key->type) {
    return -1;
  }
  if (key->type == QDICT_KEY_NUMBER) {
    return dict_key->data.num == key->data.num;
  }
  if (key->type == QDICT_KEY_STRING) {
    return (strcmp(dict_key->data.str.data, key->data.str.data) == 0);
  }
  /* NEVER reach here */
  qerror("key type %d error", key->type);
  return -1;
}

static qdict_entry_t* find(qdict_t *dict, qkey_t *key, int *save_idx) {
  int             idx;
  qlist_t        *list, *pos;
  qdict_entry_t  *entry;

  idx = mainposition(dict, key);
  if (save_idx != NULL) {
    *save_idx = idx;
  }
  list = dict->buckets[idx];
  qlist_for_each(pos, list) {
    entry = qlist_entry(pos, qdict_entry_t, entry);
    if (compare(&(entry->key), key) == 0) {
      return entry;
    }
  }
  return NULL;
}

static void copy_key(qdict_entry_t *entry, qkey_t *key) {
  qkey_t *dict_key;

  dict_key = &(entry->key);
  if (key->type == QDICT_KEY_NUMBER) {
    dict_key->type = key->type;
    dict_key->data.num = key->data.num;
    return;
  }
  if (key->type == QDICT_KEY_STRING) {
    dict_key->type = QDICT_KEY_STRING;
    qstring_null_set(&(dict_key->data.str));
    qstring_assign(&(dict_key->data.str), key->data.str.data);
    return;
  }
}

static void copy_val(qdict_entry_t *entry, qval_t *val) {
  qval_t *dict_val;
  dict_val = &(entry->val);
  memcpy(dict_val, val, sizeof(qval_t));
}

int qdict_add(qdict_t *dict, qkey_t *key, qval_t *val) {
  int             idx;
  qdict_entry_t  *entry;
  qlist_t        *list;

  if (find(dict, key, &idx) != NULL) {
    qerror("add key exist");
    return -1;
  }
  entry = qalloc(sizeof(qdict_entry_t));
  copy_key(entry, key);
  copy_val(entry, val);
  list = dict->buckets[idx];
  qlist_add_tail(&entry->entry, list);
  ++(dict->num);
  return 0;
}

int qdict_replace(qdict_t *dict, qkey_t *key, qval_t *val) {
  qdict_entry_t *entry;

  entry = find(dict, key, NULL);
  if (entry == NULL) {
    entry = qalloc(sizeof(qdict_entry_t));
    if (entry == NULL) {
      return -1;
    }
  }
  copy_key(entry, key);
  copy_val(entry, val);
  return 0;
}

qval_t* qdict_get(qdict_t *dict, qkey_t *key) {
  qdict_entry_t *entry;

  entry = find(dict, key, NULL);
  if (entry == NULL) {
    return NULL;
  }
  return &(entry->val);
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

qdict_entry_t* qdict_next(qdict_iter_t *iter) {
  qdict_t       *dict;
  qlist_t       *list, *pos;
  qdict_entry_t *entry;

  dict = iter->dict;
  if (iter->entry == NULL) {
    list = dict->buckets[iter->hash];
    /*
     * find the first non-empty hash list
     */
    while (qlist_empty(list)) {
      ++(iter->hash);
      if (iter->hash < dict->hashsize) {
        list = dict->buckets[iter->hash];
      } else {
        return NULL;
      }
    }
    entry = qlist_entry(list->next, qdict_entry_t, entry);
  } else {
    pos = iter->entry->entry.next;
    list = dict->buckets[iter->hash];
    /*
     * if reach the hash list end, get the next non-empty hash list
     */
    if (pos == list) {
      /*
       * find the first non-empty hash list
       */
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
    entry = qlist_entry(pos, qdict_entry_t, entry);
  }
  iter->entry = entry;
  return entry;
}
