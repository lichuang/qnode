/*
 * See Copyright Notice in qnode.h
 */

#ifndef __qworker_H__
#define __qworker_H__

#include <lua.h>
#include <pthread.h>
#include "qmailbox.h"
#include "qlist.h"
#include "qmsg.h"
#include "qtype.h"

struct qworker_t {
  qmailbox_t    box;
  pthread_t     id;
  qid_t        tid;
  qengine_t    *engine;
  qlist_t       actor_list;
  lua_State    *state;
};

qworker_t*  qworker_new(qid_t tid);
void        qworker_destroy(qworker_t *thread);
void        qworker_send(qid_t tid, qmsg_t *msg);

#endif  /* __qworker_H__ */
