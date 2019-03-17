/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_BASE_SINGLETON__
#define __QNODE_BASE_SINGLETON__

#include <pthread.h>
#include "base/base.h"

template <typename T>
class Singleton {
public:
  static T& instance() {
    pthread_once(&ponce_, &Singleton::init);
    return *value_;
  }

  static void init() {
    value_ = new T();
  }

private:
  DISALLOW_COPY_AND_ASSIGN(Singleton<T>);
  static pthread_once_t ponce_;
  static T* value_;
};

template<typename T>
pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;

template<typename T>
T* Singleton<T>::value_ = NULL;

#endif  // __QNODE_BASE_SINGLETON__
