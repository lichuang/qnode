/*
 * Copyright (C) codedump
 */
#ifndef __QNODE_SCRIPT_IO_THREAD_H__
#define __QNODE_SCRIPT_IO_THREAD_H__

#include "core/io_thread.h"

class LuaVM;

class ScriptIOThread : public IOThread {
public:
  ScriptIOThread(const string& name);

  void AddAllocSize(size_t s) {
    alloc_size_ += s;
  }

  virtual void Process(Message*);
private:
  LuaVM *vm_;
  size_t alloc_size_;
};

#endif  // __QNODE_SCRIPT_IO_THREAD_H__
