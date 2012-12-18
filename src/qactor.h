/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QACTOR_H__
#define __QACTOR_H__

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "qlist.h"

struct qserver_t;
struct qthread_t;

typedef struct qactor_t {
  lua_State *state;
  struct qthread_t *thread;
  int aid;
  qlist_t entry;
} qactor_t;

qactor_t *qactor_new(struct qserver_t *server);
void qactor_destroy(qactor_t *actor);

#endif  /* __QACTOR_H__ */
