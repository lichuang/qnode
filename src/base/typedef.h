/*
 * Copyright (C) codedump
 */
#ifndef __QNODE_BASE_TYPEDEF_H__
#define __QNODE_BASE_TYPEDEF_H__

#include <pthread.h>

typedef pthread_t tid_t;

typedef int fd_t;

// invalid fd const
static const fd_t kInvalidFd     = -1;

typedef pthread_key_t tls_key_t;

#endif  // __QNODE_BASE_TYPEDEF_H__
