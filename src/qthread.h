/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QTHREAD_H__
#define __QTHREAD_H__

#include <pthread.h>
#include "qlist.h"
#include "qtype.h"

struct qengine_t;
struct qmailbox_t;
struct qmsg_t;
struct qserver_t;

typedef struct qthread_t {
  unsigned short started;
  pthread_t id;
  qtid_t tid;
  struct qengine_t *engine;
  struct qmailbox_t *box;       /* server send to thread */
  struct qchannel_t **channels; /* thread to thread box */
  qlist_t actor_list;
} qthread_t;

qthread_t* qthread_new(struct qserver_t *server, qtid_t tid);
void qthread_destroy(qthread_t *thread);
struct qmailbox_t* qthread_mailbox(qthread_t *thread);

void qthread_init_thread_channel(qthread_t *src_thread, qthread_t *dst_thread);

#endif  /* __QTHREAD_H__ */
