/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QACTOR_H__
#define __QACTOR_H__

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "qcore.h"
#include "qdict.h"
#include "qtype.h"
#include "qlist.h"
#include "qmailbox.h"
#include "qmutex.h"

struct qactor_t {
  lua_State     *state;
  /*
   * lua thread YIELD reason
   */
  unsigned int    waiting_netio:1;  /* waiting for network I/O */
  unsigned int    waiting_msg:1;    /* waiting for message */
  qid_t           tid;              /* thread id which belongs to */
  qid_t           aid;              /* actor id */
  qid_t           parent;           /* parent actor id */
  qlist_t         entry;            /* entry for actor list in thread */
  qlist_t         desc_list;        /* descriptor list */
  qspinlock_t     desc_list_lock;   /* descriptor list lock */
  qlist_t         msg_list;         /* message list */
  qdict_t        *listen_params;
};

qactor_t*   qactor_new(qid_t aid);
void        qactor_attach(qactor_t *actor, lua_State *state);
void        qactor_destroy(qactor_t *actor);

/* spawn an actor as child, return the actor ID */
qid_t       qactor_spawn(qactor_t *actor, lua_State *state);

qengine_t*  qactor_get_engine(qid_t aid);
qworker_t*  qactor_get_worker(qid_t aid);

#endif  /* __QACTOR_H__ */
