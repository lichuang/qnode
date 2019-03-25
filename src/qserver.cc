/*
 * Copyright (C) codedump
 */
#include <string>
#include "base/singleton.h"
#include "core/log.h"
#include "core/server.h"

using namespace std;

#include "core/session.h"

class EchoSession : public Session {
public:
  EchoSession(int fd, const string& addr)
    : Session(fd, addr) {}

  virtual ~EchoSession() {}
  virtual void OnWrite() {
    Infof("on write");
    Session::OnWrite();
  }

  virtual void OnRead() {
    Session::OnRead();
    size_t n = socket_->Read(&buf[0], sizeof(buf));
    buf[n] = '\0';

    Infof("read client %d data: %s", n, buf);
    socket_->Write(&buf[0], n);
    socket_->SetOut();
  }

  virtual void OnError(int err) {
    Infof("session %s closed", CString());
  }

private:
  char buf[100];
};

class EchoSessionFactory : public SessionFactory {
  virtual ~EchoSessionFactory() {
  }

  virtual Session* Create(int fd, const string& addr) {
    return new EchoSession(fd, addr);
  }
};

#include "base/string.h"

int main() {
  InitLog();

  Infof("run server");

  Server *server = new Server(1);
  server->Listen("0.0.0.0", 11321, new EchoSessionFactory());
  server->Run();

  return 0;
}
