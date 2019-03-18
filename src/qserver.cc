/*
 * Copyright (C) codedump
 */

#include "base/singleton.h"
#include "core/log_thread.h"

int main() {
  gLogThread = Singleton<LogThread>::Instance();

  return 0;
}
