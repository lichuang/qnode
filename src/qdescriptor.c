/*
 * See Copyright Notice in qnode.h
 */
#include "qactor.h"
#include "qassert.h"
#include "qdescriptor.h"
#include "qmalloc.h"
#include "qserver.h"

static void init_tcp_descriptor(qdescriptor_t *desc) {
  qtcp_descriptor_t  *tcp  = &(desc->data.tcp);
  qinet_descriptor_t *inet = &(tcp->inet);
  inet->state = QINET_STATE_OPEN;
}

qdescriptor_t* qdescriptor_new(int fd, unsigned short type, qactor_t *actor) {
  qassert(fd < QID_MAX);
  qdescriptor_t *desc = g_server->descriptors[fd];
  if (desc) {
    qassert(desc);
    qassert(desc->aid == -1);
    qassert(desc->fd == fd);
  } else {
    desc = qalloc_type(qdescriptor_t);
    desc->aid = -1;
    desc->fd = fd;
    g_server->descriptors[fd] = desc;
    qlist_entry_init(&desc->entry);
  }
  desc->aid  = actor->aid;
  desc->type = type;
  qlist_add_tail(&desc->entry, &actor->desc_list);

  if (type == QDESCRIPTOR_TCP) {
    init_tcp_descriptor(desc);
  }
  return desc;
}

struct qactor_t* qdescriptor_get_actor(qdescriptor_t *desc) {
  if (desc->aid >= 0) {
    return g_server->actors[desc->aid];
  }
  return NULL;
}
