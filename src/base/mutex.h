/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_BASE_MUTEX_H__
#define __QNODE_BASE_MUTEX_H__

class Mutex {
public:
  Mutex();
  ~Mutex();

  void Lock();
  bool TryLock();
  void UnLock();
};

#endif // __QNODE_BASE_MUTEX_H__
