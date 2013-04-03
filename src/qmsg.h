/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QMSG_H__
#define __QMSG_H__

#include "qactor.h"
#include "qcore.h"
#include "qdict.h"
#include "qlist.h"
#include "qstring.h"
#include "qtype.h"

typedef struct qactor_msg_t {
  qdict_t *arg_dict;
  qlist_t entry;
  qid_t   src;
  qid_t   dst;
} qactor_msg_t;

/* 
 * s_* means server thread -> worker thread
 * w_* means worker thread -> server thread
 * t_* means worker thread -> worker thread
 * other msg can use between both side
 * */
enum {
  s_start = 1,
  s_stop  = 2,
  spawn   = 3,
  t_send  = 4,
  QMAX_MSG_TYPE
};

enum {
  SMSG_FLAG = 1,        /* server-worker message flag */
  WMSG_FLAG = 2,        /* worker-server message flag */
  TMSG_FLAG = 3,        /* worker-worker message flag */
  MSG_FLAG  = 4,        /* both side message flag */
};

/* 
 * define messages between worker-thread and main-thread 
 */
typedef struct qmsg_t {
  qlist_t entry;

  int handled:1;            /* whether the msg has been handler by receiver */
  unsigned short flag;
  unsigned int type;

  qtid_t sender_id;
  qtid_t receiver_id;

  union {
    struct {
      qid_t aid;
    } s_start;

    struct {
      qid_t aid;
      qid_t parent;
      lua_State *state;
      qactor_t *actor;
    } spawn;

    struct {
      qactor_msg_t *actor_msg;
    } t_send;
  } args;
} qmsg_t;

/* handler for worker thread msg */
typedef int (*qthread_msg_handler)(qthread_t *thread, qmsg_t *msg);

/* handler for server thread msg */
typedef int (*qserver_msg_handler)(qserver_t *server, qmsg_t *msg);

qmsg_t* qmsg_new(qtid_t sender_id, qtid_t receiver_id);
void qmsg_destroy(qmsg_t *msg);
qactor_msg_t* qactor_msg_new();
void qactor_msg_destroy(qactor_msg_t *msg);
qmsg_t* qmsg_clone(qmsg_t *msg);

void qmsg_send(qmsg_t *msg);

#define qmsg_is_smsg(msg)         ((msg)->flag == SMSG_FLAG || (msg)->flag == MSG_FLAG)
#define qmsg_is_wmsg(msg)         ((msg)->flag == WMSG_FLAG || (msg)->flag == MSG_FLAG)

#define qmsg_invalid_type(type)   ((type) <= 0 || (type) >= QMAX_MSG_TYPE)

#define qmsg_init_sstart(msg, aid)                \
  qlist_entry_init(&((msg)->entry));              \
  (msg)->type = s_start;                          \
  (msg)->flag = SMSG_FLAG;                        \
  (msg)->args.s_start.aid = (aid);                \

#define qmsg_init_sstop(msg)                      \
  qlist_entry_init(&((msg)->entry));              \
  (msg)->type = s_stop;                           \
  (msg)->flag = SMSG_FLAG;

#define qmsg_init_spawn(msg, aid, parent, state)  \
  qlist_entry_init(&((msg)->entry));              \
  (msg)->type = spawn;                            \
  (msg)->flag = MSG_FLAG;                         \
  (msg)->args.spawn.aid = (aid);                  \
  (msg)->args.spawn.parent = (parent);            \
  (msg)->args.spawn.state = (state);              \
  (msg)->args.spawn.actor = NULL

#define qmsg_init_tsend(msg, actor_msg)           \
  qlist_entry_init(&((msg)->entry));              \
  (msg)->type = t_send;                           \
  (msg)->flag = TMSG_FLAG;                        \
  (msg)->args.t_send.actor_msg = (actor_msg)               

#endif  /* __QMSG_H__ */
