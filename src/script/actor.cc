/*
 * Copyright (C) codedump
 */

#include "core/log.h"
#include "script/actor.h"
#include "script/lua_thread.h"

void
Actor::OnRead() {
  Infof("in actor");
  thread_->Resume(0);
}
