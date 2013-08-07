/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QSERVER_H__
#define __QSERVER_H__

#include "qtype.h"

int qserver_run();

void qserver_quit();

qid_t qserver_worker();

#endif  /* __QSERVER_H__ */
