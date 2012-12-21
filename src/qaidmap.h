/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QAIDMAP_H__
#define __QAIDMAP_H__

#include "qtype.h"

/* actor id allocate algorithm, refer to Linux Kernel pid map */

/* max aid, equal to 2^15=32768 */
#define AID_MAX_DEFAULT 0x8000

#define QAID_INVALID  -1

void   qaidmap_init();
void   qaidmap_destroy();
qaid_t qaid_new();
void   qaid_free(qaid_t aid);

#endif  /* __QAIDMAP_H__ */
