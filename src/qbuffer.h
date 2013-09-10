/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QBUFFER_H__
#define __QBUFFER_H__

#include <stdint.h>
#include "qcore.h"
#include "qfreelist.h"

typedef struct qbuffer_t {
  qfreeitem_fields;
  char *data;
  int   size;
  int   start;
  int   end;
} qbuffer_t;

#define qbuffer_reserve(buffer, len)  \
  if ((buffer)->size < (len)) {       \
    qbuffer_extend((buffer), (len));  \
  } 

#define qbuffer_readable(buffer)      \
  ((buffer)->data + (buffer)->start)

#define qbuffer_rlen(buffer)  \
  (buffer)->end - (buffer)->start

#define qbuffer_writeable(buffer)     \
  ((buffer)->data + (buffer)->end)

#define qbuffer_wlen(buffer) \
  (buffer)->size - (buffer)->end

#define qbuffer_reset(buffer)         \
  (buffer)->start = (buffer)->end = 0;\
  (buffer)->data[0] = '\0'

qbuffer_t*  qbuffer_new();
void        qbuffer_reinit(qbuffer_t *buffer);
void        qbuffer_free(qbuffer_t *buffer);
int         qbuffer_extend(qbuffer_t *buffer, uint32_t size);
char*       qbuffer_read(qbuffer_t *buffer, int size);
int         qbuffer_write(qbuffer_t *buffer, const char *data, int size);

int         qbuffer_init_freelist();

#endif  /* __QBUFFER_H__ */
