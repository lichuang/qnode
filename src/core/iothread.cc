/*
 * Copyright (C) codedump
 */
#include "core/iothread.h"

IOThread::IOThread()
  : dispatcher_(new Dispatcher()) {
}
