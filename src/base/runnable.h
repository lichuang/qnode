/*
 * Copyright (C) codedump
 */
#ifndef __QNODE_BASE_RUNNABLE_H__
#define __QNODE_BASE_RUNNABLE_H__

class Thread;

class Runnable {
public:
  virtual ~Runnable() {}

  virtual void Run(Thread *, void *arg) = 0;
};

#endif  // __QNODE_BASE_RUNNABLE_H__
