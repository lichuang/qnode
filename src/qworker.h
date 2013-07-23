/*
 * See Copyright Notice in qnode.h
 */

#ifndef __qworker_H__
#define __qworker_H__

#include <lua.h>
#include <pthread.h>
#include "qlist.h"
#include "qmailbox.h"
#include "qmsg.h"
#include "qtype.h"

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
  qmailbox_t            box;      /* worker msg box */
  pthread_t             id;       /* pthread id */
  qid_t                 tid;      /* worker thread id allocate by main thread */
  qid_t                 current;  /* current allocate id */
  qmutex_t              mutex;    /* current lock */
  qactor_t            **actors;   /* worker actors array */
  qengine_t            *engine;   /* dispatcher engine */
  lua_State            *state;    /* lua VM */
  int                   alloc;    /* lua memory size*/
  volatile int          running:1;
};

qworker_t*  qworker_new(qid_t tid);
void        qworker_destroy(qworker_t *thread);
void        qworker_send(qmsg_t *msg);
qid_t       qworker_new_aid(qworker_t *worker);
void        qworker_add(qid_t aid, qactor_t *actor);
void        qworker_delete(qid_t aid);
qactor_t*   qworket_get_actor(qworker_t *worker, qid_t id);

#endif  /* __qworker_H__ */
