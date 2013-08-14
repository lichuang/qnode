/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QASSERT_H__
#define __QASSERT_H__

#ifdef DEBUG

#define qassert(e) \
  if (e) { \
    ; \
  } else { \
    qassert_error(#e, __FILE__, __LINE__); \
  }

#define qalloc_assert(e) \
  ((void) ((e) ? 1 : (qassert_error("alloc memory for " #e " ", __FILE__, __LINE__), 0)))

void qassert_error(char* expr, char* file, int line);
#else

#define qassert(exp)
#define qalloc_assert(exp)

#endif

#endif /*__QASSERT_H__ */
