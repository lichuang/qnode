/*
 * See Copyright Notice in qnode.h
 */

#include "qactor.h"
#include "qalloc.h"
#include "qcore.h"
#include "qdict.h"
#include "qengine.h"
#include "qlog.h"
#include "qluautil.h"

typedef struct qltimer_t {
  lua_State  *state;

  /* actor id */
  qid_t       aid;

  /* timer id */
  qid_t       id;

  qengine_t  *engine;

  /* callback arg table */
  qdict_t    *args;

  /* mod name */
  qstring_t   mod;

  /* function name */
  qstring_t   func;

  /* free flag */
  int         flag:1;
} qltimer_t;

static qltimer_t* new_timer(qdict_t *args,
                            const char *mod,
                            const char *func);
static void free_timer(void *data);
static void engine_free_timer(void *data);

static void timer_handler(void *data);

static int qltimer_add(lua_State *state);
static int qltimer_del(lua_State *state);

luaL_Reg timer_apis[] = {
  {"qltimer_add",   qltimer_add},
  {"qltimer_del",   qltimer_del},
  {NULL,            NULL},
};

static int
qltimer_add(lua_State *state) {
  int         timeout, cycle;
  const char *mod, *func;
  qactor_t   *actor;
  qdict_t    *args;
  qengine_t  *engine;
  qid_t       id;
  qltimer_t  *timer;

  timeout = (int)lua_tonumber(state, 1);
  cycle   = (int)lua_tonumber(state, 2);
  mod     = lua_tostring(state, 3);
  func    = lua_tostring(state, 4);
  if (timeout < 0 || cycle < 0 || mod == NULL || func == NULL) {
    lua_pushnil(state);
    lua_pushliteral(state, "wrong param");
    return 2;
  }

  /* if there exist mod.fun? */
  lua_getglobal(state, mod);
  if (!lua_istable(state, -1)) {
    lua_pushnil(state);
    lua_pushfstring(state, "mod %s not exist", mod);
    return 2;
  }

  lua_getfield(state, -1, func);
  if (!lua_isfunction(state, -1)) {
    lua_pushnil(state);
    lua_pushfstring(state, "%s.%s is not lua function", mod, func);
    return 2;
  }
  /* pop the result */
  lua_pop(state, -1);

  args = qdict_new(5);
  if (args == NULL) {
    lua_pushnil(state);
    lua_pushliteral(state, "create args table error");
    return 2;
  }

  if (qlua_copy_table(state, 4, args) != QOK) {
    qdict_free(args);
    lua_pushnil(state);
    lua_pushliteral(state, "copy args table error");
    return 2;
  }

  timer = new_timer(args, mod, func);
  if (timer == NULL) {
    qdict_free(args);
    lua_pushnil(state);
    lua_pushliteral(state, "create timer error");
    return 2;
  }

  actor = qlua_get_actor(state);
  engine = qactor_get_engine(actor->aid);
  id = qengine_add_timer(engine, timeout, timer_handler,
                         free_timer, cycle, timer);
  timer->state = state;
  timer->aid   = actor->aid;
  timer->id = id;
  timer->engine = engine;
  qdict_set_numdata(actor->timers, id, timer, engine_free_timer);

  lua_pushnumber(state, id);

  return 1;
}

static int
qltimer_del(lua_State *state) {
  qid_t       id;
  qengine_t  *engine;
  qactor_t   *actor;

  id = (qid_t)lua_tonumber(state, 1);
  if (id < 0) {
    lua_pushnil(state);
    lua_pushliteral(state, "wrong param");
    return 2;
  }

  actor = qlua_get_actor(state);
  engine = qactor_get_engine(actor->aid);
  if (qdict_get_num(actor->timers, id) == NULL) {
    return 0;
  }
  qengine_del_timer(engine, id);

  return 0;
}

static void
timer_handler(void *data) {
  qltimer_t *timer;
  lua_State *state;

  timer = (qltimer_t*)data;
  state = timer->state;
  lua_getglobal(state, timer->mod);
  if (!lua_istable(state, -1)) {
    qerror("mod %s is not exist", timer->mod);
    qengine_del_timer(timer->engine, timer->id);
    return;
  }

  lua_getfield(state, -1, timer->func);
  if (!lua_isfunction(state, -1)) {
    qerror("%s.%s is not lua function", timer->mod, timer->func);
    qengine_del_timer(timer->engine, timer->id);
    return;
  }
  lua_pcall(state, 0, LUA_MULTRET, 0);
}

static qltimer_t*
new_timer(qdict_t *args, const char *mod, const char *func) {
  qltimer_t *timer;

  timer = (qltimer_t*)qalloc(sizeof(qltimer_t));
  if (timer == NULL) {
    return NULL;
  }
  timer->args = args;
  timer->mod  = qstring_new(mod);
  timer->func = qstring_new(func);
  if (timer->mod == NULL || timer->func == NULL) {
    goto error;
  }

  return timer;

error:
  if (timer->mod) {
    qstring_destroy(timer->mod);
  }
  if (timer->func) {
    qstring_destroy(timer->func);
  }
  qfree(timer);

  return NULL;
}

static void
free_timer(void *data) {
  qltimer_t *timer;

  timer = (qltimer_t*)data;
  timer->flag = 1;
  qdict_free(timer->args);
  qstring_destroy(timer->mod);
  qstring_destroy(timer->func);
  qfree(timer);
}

static void
engine_free_timer(void *data) {
  qltimer_t *timer;

  timer = (qltimer_t*)data;
  qengine_del_timer(timer->engine, timer->id);
}
