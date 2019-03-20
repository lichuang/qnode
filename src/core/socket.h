/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_CORE_SOCKET_H__
#define __QNODE_CORE_SOCKET_H__

#include "base/buffer.h"
#include "core/event.h"

class DataHandler;
class Poller;

class Socket : public Event {
public:
  Socket(int fd, DataHandler*);

  virtual ~Socket();

  void SetPoller(Poller *);

  size_t ReadBufferSize() const {
    return read_list_.TotalSize();
  }

  size_t WriteBufferSize() const {
    return write_list_.TotalSize();
  }

  void Write(const char* from, size_t n);
  void Read(char* to, size_t n);

  virtual void In();

  virtual void Out();

  virtual void Timeout();

private:
  void CloseSocket();

private:  
  int fd_;
  int error_;
  DataHandler *handler_;
  Poller* poller_;
  bool is_writable_;
  BufferList read_list_;
  BufferList write_list_;

  DISALLOW_COPY_AND_ASSIGN(Socket);
};

#endif  // __QNODE_CORE_SOCKET_H__
