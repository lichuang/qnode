/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_BASE_BUFFER_H__
#define __QNODE_BASE_BUFFER_H__

#include <string.h>
#include <list>
#include "base/base.h"
#include "base/object_pool.h"

class Buffer {
public:
  Buffer() {
    Reset();
  }

  ~Buffer() {}

  char* data() {
    return &(data_[0]);
  }

  void Reset() {
    memset(data_, '\0', sizeof(char) * kBufferSize);
  }

private:
  char data_[kBufferSize];

  DISALLOW_COPY_AND_ASSIGN(Buffer);
};

class BufferList {
public:
  BufferList(ObjectList<Buffer>* obj_list)
    : read_inx_(0)
    , write_inx_(0)
    , obj_list_(obj_list) {
    buffer_list_.push_back(obj_list->Get());
  }

  // read at most n bytes into to, return bytes actual read
  size_t Read(char* to, size_t n);

  // write n bytes into buffer list
  void Write(const char* from, size_t n);

private:
  void readAdvance(size_t n);

  void writeAdvance(size_t n);

  size_t totalSize() const {
    return (buffer_list_.size() - 1) * kBufferSize
      + write_inx_ - read_inx_;
  }

  char* readPoint() {
    return buffer_list_.front().data() + read_inx_;
  }

  char* writePoint() {
    return buffer_list_.back().data() + write_inx_;
  }

  size_t readableSize() const;
  size_t writeableSize() const;

private:
  size_t read_inx_;
  size_t write_inx_;

  std::list<Buffer*> buffer_list_;
  ObjectList<Buffer> *obj_list_;

  DISALLOW_COPY_AND_ASSIGN(BufferList);
};

#endif  // __QNODE_BASE_BUFFER_H__
