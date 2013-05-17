/*
 * See Copyright Notice in qnode.h
 */

#include "qlog.h"
#include "qsignaler.h"

int            g_caught[NSIG] = {0};
qsignal_func_t g_signal_func[NSIG] = {NULL};
static int     g_max = 0;

static void signal_handler(int signal) {
  if (g_max < signal) {
    g_max = signal;
  }
  qinfo("caught signal: %d", signal);
  g_caught[signal] = 1;
  kSignaler->Active();
}

void qsignal_add(int signal, qsignal_func_t handler) {
  struct sigaction act;

  handlers_[signal] = handler;

  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_NODEFER | SA_ONSTACK | SA_RESETHAND;
  if (handler) {
    act.sa_handler = signal_handler;
  } else {
    act.sa_handler = SIG_IGN;
  }
  sigaction(signal, &act, NULL);
}
