/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QSERVER_H__
#define __QSERVER_H__

#include "qconfig.h"
#include "qcore.h"
#include "qidmap.h"
#include "qmutex.h"
#include "qtype.h"

#define QSERVER_THREAD_TID 0

enum {
  STOPPED   = 0,
  RUNNING   = 1,
  STOPPING  = 2,
};

struct qserver_t {
  qactor_t        **actors;
  qconfig_t        *config;
  qdescriptor_t   **descriptors;
  qengine_t        *engine;
  qmailbox_t      **in_box;
  qmailbox_t      **out_box;
  qmem_pool_t      *pool;
  qthread_t       **threads;
  qthread_log_t   **thread_log;
  unsigned int      num_actor;
  qidmap_t          id_map;
  qmutex_t          id_map_mutex;
  int               status;
};

int qserver_run(struct qconfig_t *config);

void qserver_new_actor(struct qactor_t *actor);

qtid_t qserver_worker_thread();

struct qactor_t* qserver_get_actor(qid_t id);

/* the GLOBAL server in the system */
extern struct qserver_t *g_server;

#endif  /* __QSERVER_H__ */
