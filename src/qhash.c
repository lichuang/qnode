/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include <string.h>
#include "qassert.h"
#include "qhash.h"
#include "qlog.h"
#include "qmalloc.h"

static qhash_key_t dead_key = {{0}, 0};

static inline unsigned int hash_number(unsigned int key) {
  return key;
}

static unsigned int hash_string(const char *key, int len) {
  unsigned int hash = 5381;

  while (len--) {
    hash = ((hash << 5) + hash) + (*key++); /* hash * 33 + c */
  }
  return hash;
}

static unsigned int hash_function(qhash_key_t *key) {
  if (key->type == QHASH_KEY_NUMBER) {
    return hash_number(key->val.num);
  }
  return hash_string(key->val.str, strlen(key->val.str));
}

static inline int is_key_equal(qhash_key_t *k1, qhash_key_t *k2) {
  return ((k1->type == k2->type) && (k1->val.num == k2->val.num));
}

static qhash_entry_t *get_entry(qhash_t *hash) {
  qhash_entry_t *entry = NULL;
  if (qlist_empty(&(hash->free))) {
    entry = qalloc_type(qhash_entry_t);
  } else {
    qlist_t *next = hash->free.next;
    entry = qlist_entry(next, qhash_entry_t, entry);
    qlist_del_init(next);
  }
  qlist_entry_init(&(entry->entry));
  entry->key = &dead_key;
  return entry;
}

static void del_entry(qhash_t *hash, qhash_entry_t *entry) {
  qlist_del_init(&(entry->entry));
  qlist_add(&(entry->entry), &(hash->free));
  entry->key = &dead_key;
}

qhash_t* qhash_new(int hsize) {
  qhash_t *hash = qalloc_type(qhash_t);
  qalloc_assert(hash);
  hash->buckets = qalloc_array(qlist_t*, hsize);
  qalloc_assert(hash->buckets);
  int i = 0;
  for (i = 0; i < hsize; ++i) {
    qhash_entry_t *entry = get_entry(hash);
    if (entry == NULL) {
      return NULL;
    }
    hash->buckets[i] = &(entry->entry);
  }
  hash->hsize = hsize;
  hash->num = 0;
  qlist_entry_init(&(hash->free));
  return hash;
}

void qhash_destroy(qhash_t *hash) {
  qfree(hash->buckets);
  qfree(hash);
}

int qhash_add(qhash_t *hash, qhash_key_t *key, void *data) {
  qhash_entry_t *entry = get_entry(hash);
  if (entry == NULL) {
    return -1;
  }
  qlist_entry_init(&(entry->entry));
  entry->key = key;
  entry->data = data;
  unsigned int index = hash_function(key) / hash->hsize;
  qlist_t *pos = NULL;
  qhash_entry_t *node = NULL;
  qlist_for_each(pos, hash->buckets[index]) {
    node = qlist_entry(pos, qhash_entry_t, entry);
    if (is_key_equal(node->key, entry->key)) {
      qerror("key equal");
      return -1;
    }
  }
  qlist_add(&(entry->entry), hash->buckets[index]);

  return 0;
}

qhash_entry_t*  qhash_get(qhash_t *hash, qhash_key_t *key) {
  unsigned int index = hash_function(key) / hash->hsize;
  qlist_t *pos = NULL;
  qhash_entry_t *node = NULL;
  qlist_for_each(pos, hash->buckets[index]) {
    node = qlist_entry(pos, qhash_entry_t, entry);
    if (is_key_equal(node->key, key)) {
      return node;
    }
  }

  return NULL;
}

int qhash_del(qhash_t *hash, qhash_key_t *key) {
  unsigned int index = hash_function(key) / hash->hsize;
  qlist_t *pos;
  qhash_entry_t *node;
  qlist_for_each(pos, hash->buckets[index]) {
    node = qlist_entry(pos, qhash_entry_t, entry);
    if (is_key_equal(node->key, key)) {
      del_entry(hash, node);
      return 0;
    }
  }

  return -1;
}
