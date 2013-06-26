/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QTYPE_H__
#define __QTYPE_H__

typedef char*         qptr_t;
typedef unsigned int  qid_t;

/*
 * id lowest represent thread id
 * */
static const int   kMaxThreadBit = 10;

static const qid_t kMaxThreadId = 1 << kMaxThreadBit;

static const qid_t kMaxId = 0x7fffffff & (~kMaxThreadId)

#define QINVALID_ID 0

#endif  /* __QTYPE_H__ */
