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
  qlist_t  entry;
  qid_t    src;
  qid_t    dst;
} qactor_msg_t;

enum {
  START   = 1,
  SPAWN   = 2,
  SEND    = 3,
  SIGNAL  = 4,
  LOG     = 5,
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
struct qmsg_t {
  qlist_t         entry;

  unsigned int    handled:1;  /* whether the msg has been handler by receiver */
  unsigned int    type;

  qtid_t          sender_id;
  qtid_t          receiver_id;

  union {
    struct {
      qid_t aid;
    } start;

    struct {
      qid_t aid;
      qid_t parent;
      lua_State *state;
      qactor_t *actor;
    } spawn;

    struct {
      qactor_msg_t *actor_msg;
    } send;

    struct {
      qlog_t *log;
    } log;

    struct {
      int signo;
    } signal;
  } args;
};

/* handler for worker thread msg */
typedef int (*qthread_msg_handler)(qthread_t *thread, qmsg_t *msg);

/* handler for server thread msg */
typedef int (*qserver_msg_handler)(qserver_t *server, qmsg_t *msg);

qmsg_t* qmsg_new(qtid_t sender_id, qtid_t receiver_id);
void qmsg_destroy(qmsg_t *msg);
qactor_msg_t* qactor_msg_new();
void qactor_msg_destroy(qactor_msg_t *msg);
qmsg_t* qmsg_clone(qmsg_t *msg);

#define qmsg_invalid_type(type)   ((type) <= 0 || (type) >= QMAX_MSG_TYPE)

#define qmsg_init_start(msg, aid)                 \
  qlist_entry_init(&((msg)->entry));              \
  (msg)->type = START;                            \
  (msg)->args.start.aid = (aid); 

#define qmsg_init_spawn(msg, aid, parent, state)  \
  qlist_entry_init(&((msg)->entry));              \
  (msg)->type = SPAWN;                            \
  (msg)->args.spawn.aid = (aid);                  \
  (msg)->args.spawn.parent = (parent);            \
  (msg)->args.spawn.state = (state);              \
  (msg)->args.spawn.actor = NULL

#define qmsg_init_send(msg, actor_msg)            \
  qlist_entry_init(&((msg)->entry));              \
  (msg)->type = SEND;                             \
  (msg)->args.send.actor_msg = (actor_msg)               

#define qmsg_init_log(msg, log)                   \
  qlist_entry_init(&((msg)->entry));              \
  (msg)->type = LOG;                              \
  (msg)->args.log.log = (log)               

#define qmsg_init_signal(msg, sig)                \
  qlist_entry_init(&((msg)->entry));              \
  (msg)->type = SIGNAL;                           \
  (msg)->args.signal.signo = (sig)               

#endif  /* __QMSG_H__ */
