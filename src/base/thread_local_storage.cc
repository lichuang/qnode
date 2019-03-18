/*
 * Copyright (C) codedump
 */

#include "base/thread_local_storage.h"

void
CreateTLS(tls_key_t* key, void* value, void (*destructor)(void*)) {
  ::pthread_key_create(key, destructor);
  ::pthread_setspecific(*key, value);
}

void*
GetTLS(tls_key_t key) {
  return ::pthread_getspecific(key);
}
