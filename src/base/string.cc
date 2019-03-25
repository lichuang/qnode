/*
 * Copyright (C) codedump
 */

#include <iostream>
#include <sstream>
#include "base/string.h"

void Stringf(string *ret, const char *fmt, ...) {
  va_list args;
  std::stringstream ss;
  char c;
  char *s;
  int i;

  va_start(args, fmt);
  while (*fmt) {
    c = *fmt++;
    switch (c) {
    case 's':
      s = (char*)va_arg(args, char *);
      ss << s;
      break;
    case 'd':
      i = (int)va_arg(args, int);
      ss << i;
      break;
    case '%':
      break;
    default:
      ss << c;
      break;
    }
  }
  va_end(args);

  *ret = ss.str();
}
