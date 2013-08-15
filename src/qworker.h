/*
 * See Copyright Notice in qnode.h
 */

#ifndef __qworker_H__
#define __qworker_H__

#include <lua.h>
#include <pthread.h>
#include "qlimits.h"
#include "qlist.h"
#include "qmailbox.h"
#include "qmsg.h"
#include "qtype.h"
#ifdef DEBUG
#include "ldb.h"
#endif

/* max bit for actor id */
#define MAX_BIT sizeof(qid_t) * 8

/* qid_t highest ID_BIT for id */
#define ID_BIT  16

/* qid_t lowest PID_BIT for pid */
#define PID_BIT (MAX_BIT - ID_BIT)

#define POS_ID  PID_BIT

#define MAX_PID (1 << PID_BIT)
#define MAX_ID  (1 << ID_BIT)

/* creates a mask with `n' 1 bits at position `p' */
#define MASK1(n,p)  (qid_t)((~((~(qid_t)0) << (n))) << (p))

/* creates a mask with `n' 0 bits at position `p' */
#define MASK0(n,p)  (~MASK1(n,p))

/* encode aid from id and pid, where pid in the lowest bit */
#define encode_aid(id, pid) (((id) << ID_BIT) | (pid))

/* decode id from aid */
//#define decode_id(aid) (((aid) & MASK1(ID_BIT, POS_ID)) >> ID_BIT)
#define decode_id(aid) ((aid)  >> ID_BIT)

/* decode pid from aid */
#define decode_pid(aid) ((aid) & MASK1(PID_BIT, 0))

struct qworker_t {
  /* worker msg box */
  qmailbox_t            box;

  /* pthread id */
  pthread_t             id;

  /* worker thread id allocate by main thread */
  qid_t                 tid;

  /* current allocate id */
  qid_t                 current;

  /* current lock */
  qmutex_t              mutex;

  /* worker actors array */
  qactor_t            **actors;

  /* dispatcher engine per thread */
  qengine_t            *engine;

  /* lua VM */
  lua_State            *state;

  /* lua memory size*/
  int                   alloc;

  /* worker active actor list */
  qlist_t               actor_list;

  /* thread running flag */
  volatile int          running:1;

  /* lua debugger */
#ifdef DEBUG
  ldb_t                *ldb;
#endif
};

qworker_t*  qworker_new(qid_t tid);
void        qworker_destroy(qworker_t *thread);
void        qworker_send(qmsg_t *msg);
qid_t       qworker_new_aid(qworker_t *worker);
void        qworker_add(qid_t aid, qactor_t *actor);
void        qworker_delete(qid_t aid);
qactor_t*   qworket_get_actor(qworker_t *worker, qid_t id);

extern qworker_t* workers[QMAX_WORKER];

#endif  /* __qworker_H__ */
