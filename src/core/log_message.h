/*
 * Copyright (C) codedump
 */
#include "core/message.h"

#include <string>
using namespace std;

class LogThread;

class LogMessage : public Message {
  friend class LogThread;
public:
  LogMessage(int level, const char* file, int line, const char *format, ...);

  virtual ~LogMessage() {
  }

private:  
  int level_;
  string file_;
  int line_;
  char buffer_[1024];
};

