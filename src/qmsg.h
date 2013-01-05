/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QMSG_H__
#define __QMSG_H__

#include "qactor.h"
#include "qlist.h"
#include "qtype.h"

#define QMSG_MASK_UNDELETED 0x1

struct qmailbox_t;

/* 
 * s_* means server thread -> worker thread
 * w_* means worker thread -> server thread
 * t_* means worker thread -> worker thread
 * other msg can use between both side
 * NOTE: actor MUST be create in server thread and send to worker thread
 * */
enum {
  w_thread_started = 1, /* worker thread send when thread started */
  s_init  = 2,          /* server thread send when all worker thread started */
  w_thread_box = 3,     /* worker thread send its mailbox to other worker thread */
  s_start = 4,
  spawn   = 5,
  QMAX_MSG_TYPE
};

enum {
  SMSG_FLAG = 1,
  WMSG_FLAG = 2,
  MSG_FLAG  = 3,
};

/* define messages between worker-thread and main-thread */
typedef struct qmsg_t {
  qlist_t entry;

  unsigned short flag;
  unsigned int type;

  qtid_t sender_id;
  qtid_t receiver_id;

  unsigned int mask;

  union {
    struct {
      struct qactor_t *actor;
    } s_start;

    struct {
      qid_t aid;
      qid_t parent;
      lua_State *state;
      qactor_t *actor;
    } spawn;
  } args;
} qmsg_t;

/* handler for server thread msg */
typedef int (*smsg_handler)(struct qthread_t *thread, struct qmsg_t *msg);

/* handler for worker thread msg */
typedef int (*wmsg_handler)(struct qserver_t *server, struct qmsg_t *msg);

qmsg_t* qmsg_new(qtid_t sender_id, qtid_t receiver_id);

#define qmsg_is_smsg(msg)         ((msg)->flag == SMSG_FLAG || (msg)->flag == MSG_FLAG)
#define qmsg_is_wmsg(msg)         ((msg)->flag == WMSG_FLAG || (msg)->flag == MSG_FLAG)

#define qmsg_invalid_type(type)   ((type) <= 0 || (type) >= QMAX_MSG_TYPE)

#define qmsg_mark(msg, flag)      ((msg)->mask |= (flag))
#define qmsg_clearmark(msg, flag) ((msg)->mask &= ~(flag))
#define qmsg_checkmark(msg, flag) ((msg)->mask & (flag))

#define qmsg_set_undelete(msg)    qmsg_mark(msg, QMSG_MASK_UNDELETED)

#define qmsg_clear_undelete(msg)  qmsg_clearmark(msg, QMSG_MASK_UNDELETED)

#define qmsg_undelete(msg)        qmsg_checkmark(msg, QMSG_MASK_UNDELETED)

#define qmsg_init_thread_start(msg)               \
do {                                              \
  qlist_entry_init(&(msg->entry));                \
  msg->type = w_thread_started;                   \
  msg->flag = WMSG_FLAG;                          \
} while (0)

#define qmsg_init_sstart(msg, actor)              \
do {                                              \
  qlist_entry_init(&(msg->entry));                \
  msg->type = s_start;                            \
  msg->flag = SMSG_FLAG;                          \
  msg->args.s_start.actor = (actor);              \
} while (0)

#define qmsg_init_spawn(msg, aid, parent, state)  \
do {                                              \
  qlist_entry_init(&(msg->entry));                \
  msg->type = spawn;                              \
  msg->flag = MSG_FLAG;                           \
  msg->args.spawn.aid = (aid);                    \
  msg->args.spawn.parent = (parent);              \
  msg->args.spawn.state = (state);                \
  msg->args.spawn.actor = NULL;                   \
} while (0)

#endif  /* __QMSG_H__ */
