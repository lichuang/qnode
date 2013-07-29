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

struct qsocket_t {
  qfree_item_fields;

  /* in buffer */
  qbuffer_t       *in;

  /* out buffer */
  qbuffer_t       *out;

  /* if accept socket */
  unsigned         accept:1;

  int              fd;

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
