/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QTHREAD_H__
#define __QTHREAD_H__

#include <lua.h>
#include <pthread.h>
#include "qacceptor.h"
#include "qlist.h"
#include "qmsg.h"
#include "qtype.h"

struct qthread_t {
  qacceptor_t   acceptor;
  pthread_t     id;
  qtid_t        tid;
  qengine_t    *engine;
  qlist_t       actor_list;
  lua_State    *state;
};

qthread_t*  qthread_new(qtid_t tid);
void        qthread_destroy(qthread_t *thread);
void        qthread_send(qtid_t tid, qmsg_t *msg);

extern qthread_msg_handler g_thread_msg_handlers[];

#endif  /* __QTHREAD_H__ */
