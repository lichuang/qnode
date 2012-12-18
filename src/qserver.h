/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QSERVER_H__
#define __QSERVER_H__

struct qconfig_t;
struct qengine_t;
struct qevent_t;
struct qthread_t;
struct qmailbox_t;

typedef struct qserver_t {
  struct qconfig_t *config;
  struct qengine_t *engine;
  struct qthread_t  **threads;
  struct qmailbox_t **boxs;
  struct qmailbox_t **thread_boxs;
} qserver_t;

qserver_t* qserver_create(struct qconfig_t *config);
struct qmailbox_t *qserver_get_box(qserver_t *server, int tid);

#endif  /* __QSERVER_H__ */
