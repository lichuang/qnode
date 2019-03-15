/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_CORE_EVENT_H__
#define __QNODE_CORE_EVENT_H__

class Dispatcher;

static const int kEventRead  = 1 << 0;
static const int kEventWrite = 1 << 1;
static const int kEventError = 1 << 2;

class Event {
public:
  Event(int fd)
    : fd_(fd) {}

  virtual ~Event();

  int Fd() const {
    return fd_;
  }

  void SetFlags(int flags) {
    flags_ = flags;
  }
  int Flags() const {
    return flags_;
  }

  virtual void Write() = 0;
  virtual void Read()  = 0;

protected:
  int fd_;
  int flags_;
  Dispatcher *dispatcher_;
};

#endif  // __QNODE_CORE_EVENT_H__
