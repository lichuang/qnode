/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QMSG_H__
#define __QMSG_H__

#include "qactor.h"
#include "qlist.h"
#include "qtype.h"

#define QMSG_MASK_UNDELETED 0x1

/* define messages between worker-thread and main-thread */
typedef struct qmsg_t {
  qlist_t entry;

  enum {
    SMSG_FLAG = 1,
    WMSG_FLAG = 2,
    MSG_FLAG  = 3,
  } flag;

  /* 
   * s_* means server thread -> worker thread
   * t_* means worker thread -> server thread
   * other msg can use both
   * NOTE: actor MUST be create in server thread and send to worker thread
   * */
  enum {
    s_start = 1,
    spawn,
    MAX_MSG_TYPE
  } type;
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

qmsg_t* qmsg_new();

#define qmsg_is_smsg(msg)         ((msg)->flag == SMSG_FLAG || (msg)->flag == MSG_FLAG)
#define qmsg_is_wmsg(msg)         ((msg)->flag == WMSG_FLAG || (msg)->flag == MSG_FLAG)

#define qmsg_invalid_type(type)   ((type) <= 0 || (type) >= MAX_MSG_TYPE)

#define qmsg_mark(msg, flag)      ((msg)->mask |= (flag))
#define qmsg_clearmark(msg, flag) ((msg)->mask &= ~(flag))
#define qmsg_checkmark(msg, flag) ((msg)->mask & (flag))

#define qmsg_set_undelete(msg)    qmsg_mark(msg, QMSG_MASK_UNDELETED)

#define qmsg_clear_undelete(msg)  qmsg_clearmark(msg, QMSG_MASK_UNDELETED)

#define qmsg_undelete(msg)        qmsg_checkmark(msg, QMSG_MASK_UNDELETED)

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
