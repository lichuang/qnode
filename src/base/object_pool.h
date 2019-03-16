/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_BASE_OBJECT_POOL_H__
#define __QNODE_BASE_OBJECT_POOL_H__

#include <list>
#include "base/base.h"

// non-thread safe object pool

template <typename T>
class ObjectPool {
public:
  ObjectPool();

  ~ObjectPool();

  T* Get();
  void Free(T *obj);

private:
  typedef std::list<T*> ObjectList;
  typedef ObjectList::iterator ObjectListIter;

  ObjectList free_list_;

  DISALLOW_COPY_AND_ASSIGN(ObjectPool<T>);
};

#endif  // __QNODE_BASE_OBJECT_POOL_H__
