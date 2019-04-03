/*
 * Copyright (C) codedump
 */

#include "core/log.h"
#include "core/accept_message.h"
#include "script/actor.h"
#include "script/lua_thread.h"
#include "script/lua_vm.h"
#include "script/script_io_thread.h"

ScriptIOThread::ScriptIOThread(const string& name)
  : IOThread(name),
    alloc_size_(0) {
  vm_ = new LuaVM("", this);
}

void
ScriptIOThread::Process(Message *msg) {
  int type = msg->Type();

  if (type == kAcceptMessage) {
    AcceptMessage* am = dynamic_cast<AcceptMessage*>(msg);
    if (am == NULL) {
      return;
    }
    Session* session = am->GetSession();
    Actor *actor = dynamic_cast<Actor*>(session);
    if (actor == NULL) {
      return;
    }
    LuaThread *thread = new LuaThread(vm_, actor);
    actor->SetLuaThread(thread);
    actor->SetPoller(poller_);
    Infof("process connection from %s", session->String().c_str());
  }
}
