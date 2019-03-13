/*
 * Copyright (C) codedump
 */

#include "base/base.h"
#include "base/object_pool.h"

template <typename T>
ObjectPool::ObjectPool() {
  allocate();
}

template <typename T>
ObjectPool::~ObjectPool() {
  ObjectListIter iter = free_list_.begin();

  while (iter != free_list_.end()) {
    T *obj = *iter;
    ++iter;
    delete obj;
  }
}

template <typename T>
void ObjectPool::allocate() {
  int i;

  for (i = 0; i < kAllocObjectNumber; ++i) {
    T* obj = new T();
    if (obj != NULL) {
      free_list_.push_back(obj);
    }
  }
}

template <typename T>
T* ObjectPool::Get() {
  if (free_list_.empty()) {
    allocate();
  }
  T* obj = free_list_.front();
  free_list_.pop_front();
  return obj;
}

template <typename T>
void ObjectPool::Free(T *obj) {
  obj->Reset();
  free_list_.push_back(obj);
}
