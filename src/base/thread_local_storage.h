/*
 * Copyright (C) codedump
 */
#ifndef __QNODE_BASE_THREAD_LOCAL_STORAGE_H__
#define __QNODE_BASE_THREAD_LOCAL_STORAGE_H__

#include "base/typedef.h"

extern void  CreateTLS(tls_key_t*, void*, void (*destructor)(void*));
extern void* GetTLS(tls_key_t);

#endif  // __QNODE_BASE_THREAD_LOCAL_STORAGE_H__
