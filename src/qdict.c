/*
 * See Copyright Notice in qnode.h
 */

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <string.h>
#include "qassert.h"
#include "qdict.h"
#include "qlog.h"
#include "qmalloc.h"

qdict_t* qdict_new(unsigned int hashsize) {
  int i;
  qdict_t *dict = qalloc_type(qdict_t);
  dict->hashsize = hashsize;
  dict->num = 0;
  dict->buckets  = (qlist_t **)qmalloc(hashsize * sizeof(qlist_t*));
  for (i = 0; i < (int)hashsize; ++i) {
    qlist_t *list = qalloc_type(qlist_t);
    qlist_entry_init(list);
    dict->buckets[i] = list;
  }
  return dict;
}

void qdict_destroy(qdict_t *dict) {
  int i;
  for (i = 0; i < (int)dict->hashsize; ++i) {
    qlist_t *list = dict->buckets[i];
    qlist_t *pos, *next;
    for (pos = list->next; pos != list; ) {
      qdict_entry_t *entry = qlist_entry(pos, qdict_entry_t, entry);
      next = pos->next;
      if (entry->key.type == QDICT_KEY_STRING) {
        qstring_destroy(entry->key.data.str);
      }
      if (entry->val.type == QDICT_VAL_STRING) {
        qstring_destroy(entry->val.data.str);
      }
      qfree(entry);
      pos  = next;
    }
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
    return hashstring(key->data.str, strlen(key->data.str)) % dict->hashsize;
  }
  /* NEVER reach here */
  qerror("key type %d error", key->type);
  return -1;
}

static int compare(qdict_key_t *dict_key, qkey_t *key) {
  if (dict_key->type != key->type) {
    return -1;
  }
  if (key->type == QDICT_KEY_NUMBER) {
    return dict_key->data.num == key->data.num;
  }
  if (key->type == QDICT_KEY_STRING) {
    return (strcmp(dict_key->data.str->data, key->data.str) == 0);
  }
  /* NEVER reach here */
  qerror("key type %d error", key->type);
  return -1;
}

static qdict_entry_t* find(qdict_t *dict, qkey_t *key, int *save_idx) {
  int idx = mainposition(dict, key);
  if (save_idx != NULL) {
    *save_idx = idx;
  }
  qlist_t *list = dict->buckets[idx];
  qlist_t *pos = NULL;
  qdict_entry_t *entry;
  qlist_for_each(pos, list) {
    entry = qlist_entry(pos, qdict_entry_t, entry);
    if (compare(&(entry->key), key) == 0) {
      return entry;
    }
  }
  return NULL;
}

static void copy_key(qdict_entry_t *entry, qkey_t *key) {
  qdict_key_t *dict_key = &(entry->key);
  if (key->type == QDICT_KEY_NUMBER) {
    dict_key->type = key->type;
    dict_key->data.num = key->data.num;
    return;
  }
  if (key->type == QDICT_KEY_STRING) {
    dict_key->type = QDICT_KEY_STRING;
    dict_key->data.str  = qstring_new();
    qstring_assign(dict_key->data.str, key->data.str);
    return;
  }
}

static void copy_val(qdict_entry_t *entry, qdict_val_t *val) {
  qdict_val_t *dict_val = &(entry->val);
  memcpy(dict_val, val, sizeof(qdict_val_t));
}

int qdict_add(qdict_t *dict, qkey_t *key, qdict_val_t *val) {
  int idx;
  if (find(dict, key, &idx) != NULL) {
    qerror("add key exist");
    return -1;
  }
  qdict_entry_t *entry = qalloc_type(qdict_entry_t);
  copy_key(entry, key);
  copy_val(entry, val);
  qlist_t *list = dict->buckets[idx];
  qlist_add_tail(&entry->entry, list);
  dict->num = 0;
  return 0;
}

int qdict_replace(qdict_t *dict, qkey_t *key, qdict_val_t *val) {
  qdict_entry_t *entry = find(dict, key, NULL);
  if (find(dict, key, NULL) == NULL) {
    entry = qalloc_type(qdict_entry_t);
  }
  copy_key(entry, key);
  copy_val(entry, val);
  return 0;
}

qdict_val_t* qdict_get(qdict_t *dict, qkey_t *key) {
  qdict_entry_t *entry = find(dict, key, NULL);
  if (entry == NULL) {
    return NULL;
  }
  return &(entry->val);
}

qdict_iter_t* qdict_iterator(qdict_t *dict) {
  qdict_iter_t *iter = qalloc_type(qdict_iter_t);
  iter->dict  = dict;
  iter->hash  = 0;
  iter->entry = NULL;
  return iter;
}

qdict_entry_t* qdict_next(qdict_iter_t *iter) {
  qdict_t *dict = iter->dict;
  qdict_entry_t *entry = NULL;
  if (iter->entry == NULL) {
    qlist_t *list = dict->buckets[iter->hash];
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
    qlist_t *pos = iter->entry->entry.next;
    qlist_t *list = dict->buckets[iter->hash];
    /*
     * if reach the hash list end, get the next non-empty hash list
     */
    if (pos->next == list) {
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
