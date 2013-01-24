/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QDESCRIPTOR_H__
#define __QDESCRIPTOR_H__

#include "qlist.h"
#include "qtype.h"

struct qactor_t;

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
  qinet_descriptor_t inet;
} qtcp_descriptor_t;

typedef struct qfile_descriptor_t {
} qfile_descriptor_t;

#define QDESCRIPTOR_TCP   1
#define QDESCRIPTOR_FILE  2 

typedef struct qdescriptor_t {
  int fd;
  unsigned short type;
  qid_t aid;
  qlist_t entry;

  union {
    struct qtcp_descriptor_t  tcp;
    struct qfile_descriptor_t file;
  } desc;
} qdescriptor_t;

qdescriptor_t* qdescriptor_new(int fd, unsigned short type, qactor_t *actor);

#endif  /* __QDESCRIPTOR_H__ */
