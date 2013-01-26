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

struct qengine_t;
struct qmailbox_t;
struct qmsg_t;
struct qserver_t;
struct qthread_t;

typedef struct qthread_box_t {
  struct qthread_t *thread;
  struct qmailbox_t *box;
} qthread_box_t;

typedef struct qthread_t {
  unsigned short started;
  pthread_t id;
  qtid_t tid;
  struct qengine_t *engine;
  struct qmailbox_t **in_box;
  struct qmailbox_t **out_box;
  qthread_box_t **thread_box;
  qlist_t actor_list;
  lua_State *state;
} qthread_t;

qthread_t* qthread_new(struct qserver_t *server, qtid_t tid);
void qthread_destroy(qthread_t *thread);

void qthread_init_thread_channel(qthread_t *src_thread, qthread_t *dst_thread);

extern qthread_msg_handler g_thread_msg_handlers[];
#endif  /* __QTHREAD_H__ */
