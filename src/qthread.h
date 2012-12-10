/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QTHREAD_H__
#define __QTHREAD_H__

#include <pthread.h>

struct qengine_t;
struct qmailbox_t;
struct qserver_t;

typedef struct qthread_t {
  pthread_t id;
  int tid;
  struct qengine_t *engine;
  struct qmailbox_t *box;        /* server send to thread */
  struct qmailbox_t *server_box; /* thread send to server */
  struct qserver_t  *server;
} qthread_t;

qthread_t* qthread_new(struct qserver_t *server, int tid);
void qthread_destroy(qthread_t *thread);
struct qmailbox_t* qthread_mailbox(qthread_t *thread);

#endif  /* __QTHREAD_H__ */
