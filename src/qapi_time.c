/*
 * See Copyright Notice in qnode.h
 */

#include "qactor.h"
#include "qalloc.h"
#include "qcore.h"
#include "qdict.h"
#include "qengine.h"
#include "qluautil.h"

typedef struct qltimer_t {
  qdict_t *args;
  int      idx;
} qltimer_t;

static qltimer_t* new_timer(int idx, qdict_t *args);
static void       free_timer(void *data);

static void timer_handler(void *data);

static int qltimer_add(lua_State *state);
static int qltimer_del(lua_State *state);

luaL_Reg time_apis[] = {
  {"qltimer_add",   qltimer_add},
  {"qltimer_del",   qltimer_del},
};

static int
qltimer_add(lua_State *state) {
  int         timeout, cycle, idx;
  qactor_t   *actor;
  qdict_t    *args;
  qengine_t  *engine;
  qid_t       id;
  qltimer_t  *timer;

  timeout = (int)lua_tonumber(state, 1);
  cycle   = (int)lua_tonumber(state, 2);
  idx     = (int)lua_tonumber(state, 3);
  if (timeout < 0 || cycle < 0 || idx < 0) {
    lua_pushnil(state);
    lua_pushliteral(state, "wrong param");
    return 2;
  }

  args = qdict_new(5);
  if (args == NULL) {
    lua_pushnil(state);
    lua_pushliteral(state, "create args table error");
    return 2;
  }

  if (qlua_copy_table(state, 4, args) != QOK) {
    qdict_destroy(args);
    lua_pushnil(state);
    lua_pushliteral(state, "copy args table error");
    return 2;
  }

  timer = new_timer(idx, args);
  if (timer == NULL) {
    qdict_destroy(args);
    lua_pushnil(state);
    lua_pushliteral(state, "create timer error");
    return 2;
  }

  actor = qlua_get_actor(state);
  engine = qactor_get_engine(actor->aid);
  id = qengine_add_timer(engine, timeout, timer_handler,
                         free_timer, cycle, timer);

  lua_pushnumber(state, id);

  return 1;
}

static int
qltimer_del(lua_State *state) {
  qengine_t  *engine;
  qid_t       id;
  qactor_t   *actor;

  id = (qid_t)lua_tonumber(state, 1);
  if (id < 0) {
    lua_pushnil(state);
    lua_pushliteral(state, "wrong param");
    return 2;
  }

  actor = qlua_get_actor(state);
  engine = qactor_get_engine(actor->aid);
  qengine_del_timer(engine, id);

  return 0;
}

static void
timer_handler(void *data) {
  qltimer_t *timer;

  timer = (qltimer_t*)data;
}

static qltimer_t*
new_timer(int idx, qdict_t *args) {
  qltimer_t *timer;

  timer = (qltimer_t*)qalloc(sizeof(qltimer_t));
  if (timer == NULL) {
    return NULL;
  }
  timer->args = args;
  timer->idx  = idx;

  return timer;
}

static void
free_timer(void *data) {
  qltimer_t *timer;

  timer = (qltimer_t*)data;
  qdict_destroy(timer->args);
  qfree(timer);
}
