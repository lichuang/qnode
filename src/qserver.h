/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QSERVER_H__
#define __QSERVER_H__

#include "qtype.h"

struct qactor_t;
struct qconfig_t;
struct qengine_t;
struct qevent_t;
struct qthread_t;
struct qmailbox_t;
struct qmsg_t;

typedef struct qserver_t {
  struct qconfig_t *config;
  struct qengine_t *engine;
  struct qthread_t  **threads;
  struct qmailbox_t **box;
  struct qmailbox_t **thread_box;
  struct qactor_t **actors;
} qserver_t;

int qserver_run(struct qconfig_t *config);

/* worker thread tid send mail to server thread */
int qserver_add_mail(qtid_t tid, struct qmsg_t *msg);

/* server thread send mail to worker thread */
void qserver_send_mail(struct qmsg_t *msg);

/* the GLOBAL server in the system */
extern struct qserver_t *g_server;

#endif  /* __QSERVER_H__ */
