/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QSERVER_H__
#define __QSERVER_H__

#include "qconfig.h"
#include "qcore.h"
#include "qidmap.h"
#include "qmailbox.h"
#include "qmutex.h"
#include "qtype.h"

struct qserver_t {
  qmailbox_t        box;
  qconfig_t        *config;       /* server config */
  qdescriptor_t   **descriptors;  /* descriptor array */
  qengine_t        *engine;       /* event dispatch engine */
  qworker_t       **workers;      /* worker threads array */
  qthread_log_t   **thread_log;   /* thread log array(include main) */
};

int qserver_run(qconfig_t *config);

void qserver_worker_started();

qid_t qserver_worker();

/* the GLOBAL server in the system */
extern qserver_t *server;

#endif  /* __QSERVER_H__ */
