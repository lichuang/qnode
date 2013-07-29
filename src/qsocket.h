/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QSOCKET_H__
#define __QSOCKET_H__

#include <ifaddrs.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "qcore.h"
#include "qbuffer.h"
#include "qfreelist.h"
#include "qlist.h"
#include "qtype.h"

#define QINET_F_OPEN         0x0001
#define QINET_F_BOUND        0x0002
#define QINET_F_ACTIVE       0x0004
#define QINET_F_LISTEN       0x0008
#define QINET_F_CON          0x0010
#define QINET_F_ACC          0x0020
#define QINET_F_LST          0x0040
#define QINET_F_BUSY         0x0080 
#define QINET_F_MULTI_CLIENT 0x0100 /* Multiple clients for one descriptor, i.e. multi-accept */

#define QINET_STATE_CLOSED          (0)
#define QINET_STATE_OPEN            (QINET_F_OPEN)
#define QINET_STATE_BOUND           (QINET_STATE_OPEN | QINET_F_BOUND)
#define QINET_STATE_CONNECTED       (QINET_STATE_BOUND | QINET_F_ACTIVE)
#define QINET_STATE_LISTENING       (QINET_STATE_BOUND | QINET_F_LISTEN)
#define QINET_STATE_CONNECTING      (QINET_STATE_BOUND | QINET_F_CON)
#define QINET_STATE_ACCEPTING       (QINET_STATE_LISTENING | QINET_F_ACC)
#define QINET_STATE_MULTI_ACCEPTING (QINET_STATE_ACCEPTING | QINET_F_MULTI_CLIENT)

struct qsocket_t {
  qfree_item_fields;

  /* in buffer */
  qbuffer_t       *in;

  /* out buffer */
  qbuffer_t       *out;

  /* if accept socket */
  unsigned         accept:1;

  int              fd;

  /* actor desc list */
  qlist_t         entry;

  /* socket state */
  int              state;

  /* owner actor id */
  qid_t            aid;

  struct sockaddr  remote;
  char             addr[15];
  char             peer[15];
  int              port;
};

void       qsocket_init_free_list();
void       qsocket_destroy_free_list();
qsocket_t* qsocket_new(int fd, qactor_t *actor);
void       qsocket_free(qsocket_t *socket);

#endif /* __QSOCKET_H__ */
