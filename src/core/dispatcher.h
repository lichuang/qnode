/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_CORE_DISPATCHER_H__
#define __QNODE_CORE_DISPATCHER_H__

#include <string>

using namespace std;

class Event;

// epoll
class Epoll;
class EpollEntry;

typedef Epoll       Poller;
typedef EpollEntry  Handle;

class Dispatcher {
};

#endif  // __QNODE_CORE_DISPATCHER_H__
