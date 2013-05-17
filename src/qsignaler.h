/*
 * See Copyright Notice in qnode.h
 */

#ifndef __QSIGNALER_H__
#define __QSIGNALER_H__

#include <signal.h>

struct qsignaler {

};

typedef void (*qsignal_func_t)(int, void *);

extern int            g_caught[NSIG];
extern qsignal_func_t g_signal_func[NSIG];

void qsignal_add(int signal, qsignal_func_t);

#endif  /* __QSIGNALER_H__ */
