/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QTHREAD_H__
#define __QTHREAD_H__

#include <lua.h>
#include <pthread.h>
#include "qlist.h"
#include "qmsg.h"
#include "qtype.h"

typedef struct qthread_box_t {
  qmailbox_t         *box;
  qthread_t          *thread;
} qthread_box_t;

struct qthread_t {
  pthread_t           id;
  qtid_t              tid;
  qengine_t          *engine;
  qmailbox_t        **in_box;
  qmailbox_t        **out_box;
  qthread_box_t     **thread_box;
  qlist_t             actor_list;
  lua_State          *state;
};

qthread_t*  qthread_new(qserver_t *server, qtid_t tid);
void        qthread_destroy(qthread_t *thread);

extern qthread_msg_handler g_thread_msg_handlers[];

#endif  /* __QTHREAD_H__ */
