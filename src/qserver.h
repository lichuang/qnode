/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QSERVER_H__
#define __QSERVER_H__

#include "qcore.h"
#include "qtype.h"

struct qserver_t {
  //qdescriptor_t   **descriptors;  /* descriptor array */
};

int qserver_run();

void qserver_quit();

qid_t qserver_worker();

/* the GLOBAL server in the system */
extern qserver_t *server;

#endif  /* __QSERVER_H__ */
