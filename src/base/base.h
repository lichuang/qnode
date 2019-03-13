/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_BASE__
#define __QNODE_BASE__

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

// size per buffer
static const int kBufferSize = 4 * 1024;

// number per ObjectPool alloc
static const int kAllocObjectNumber = 100;

#endif  // __QNODE_BASE__
