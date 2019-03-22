/*
 * Copyright (C) codedump
 */

#include "base/thread_local_storage.h"

void
CreateTLSKey(tls_key_t *key, void (*destructor)(void*)) {
  *key = PTHREAD_ONCE_INIT;
  ::pthread_key_create(key, destructor);
}

void
CreateTLS(tls_key_t key, void* value) {
  ::pthread_setspecific(key, value);
}

void*
GetTLS(tls_key_t key) {
  return ::pthread_getspecific(key);
}
