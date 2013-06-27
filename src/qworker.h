/*
 * See Copyright Notice in qnode.h
 */

#ifndef __qworker_H__
#define __qworker_H__

#include <lua.h>
#include <pthread.h>
#include "qhandler.h"
#include "qlist.h"
#include "qmailbox.h"
#include "qmsg.h"
#include "qtype.h"

#define MAX_BIT sizeof(qid_t)
#define ID_BIT  16
#define PID_BIT (MAX_BIT - ID_BIT)
#define POS_ID  PID_BIT

#define MAX_PID (1 << PID_BIT)
#define MAX_ID  (1 << ID_BIT)

/* creates a mask with `n' 1 bits at position `p' */
#define MASK1(n,p)  ((~((~(qid_t)0) << n)) << p)

/* creates a mask with `n' 0 bits at position `p' */
#define MASK0(n,p)  (~MASK1(n,p))

#define encode_handler_id(id, pid) \
  (((id) << ID_BIT) | (pid))

#define decode_id(handler_id) \
  (((handler_id) & MASK1(ID_BIT, POS_ID)) >> ID_BIT)

#define decode_pid(handler_id) \
  ((handler_id) & MASK1(PID_BIT, 0))

struct qworker_t {
  qmailbox_t            box;
  qhandler_manager_t    manager;
  pthread_t             id;
  qid_t                 tid;
  qengine_t            *engine;
  qlist_t               actor_list;
  lua_State            *state;
};

qworker_t*  qworker_new(qid_t tid);
void        qworker_destroy(qworker_t *thread);
void        qworker_send(qmsg_t *msg);

#endif  /* __qworker_H__ */
