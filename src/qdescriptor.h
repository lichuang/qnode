/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QDESCRIPTOR_H__
#define __QDESCRIPTOR_H__

#include <ifaddrs.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "qcore.h"
#include "qbuffer.h"
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

typedef struct qinet_descriptor_t {
  int state;
} qinet_descriptor_t;

typedef struct qtcp_descriptor_t {
  qinet_descriptor_t  inet;
  qbuffer_t           buffer;
  struct sockaddr     remote;
  char                addr[50];
  int                 port;
} qtcp_descriptor_t;

typedef struct qfile_descriptor_t {
} qfile_descriptor_t;

#define QDESCRIPTOR_TCP   1
#define QDESCRIPTOR_FILE  2 

struct qdescriptor_t {
  int             fd;
  unsigned        type;
  qid_t           aid;              /* owner actor id */
  qlist_t         entry;            /* actor desc list */

  union {
    struct qtcp_descriptor_t  tcp;
    struct qfile_descriptor_t file;
  } data;
};

qdescriptor_t*    qdescriptor_new(int fd, unsigned short type, qactor_t *actor);
void              qdescriptor_destroy(qdescriptor_t *desc);
qactor_t*         qdescriptor_get_actor(qdescriptor_t *desc);

#endif  /* __QDESCRIPTOR_H__ */
