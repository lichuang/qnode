/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_CORE_SERVER_H__
#define __QNODE_CORE_SERVER_H__

#include <map>
#include <vector>
#include "base/base.h"
#include "core/acceptor_handler.h"

using namespace std;

class Listener;
class IOThread;
class Poller;
class Session;
class SessionFactory;

class Server : public AcceptorHandler {
public:
  Server(int worker);
  ~Server();

  virtual void OnAccept(Session*);
  virtual void OnError(int err);

  void Listen(const string& addr, int port, SessionFactory*);
  void Run();
private:
  int index_;
  vector<IOThread*> workers_;
  Poller *poller_;
  map<string, Listener*> listeners_;

  DISALLOW_COPY_AND_ASSIGN(Server);
};

#endif  // __QNODE_CORE_SERVER_H__
