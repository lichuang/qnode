#include "qhandler.h"

void
qhandler_manager_init(qhandler_manager_t *mng) {
  mng->current = 0;
}

qid_t
qhandler_add(qhandler_manager_t *mng,qhandler_t *handler) {
  qid_t current;

  current = mng->current;
  while (mng->handlers[current]) {
    current = current + 1;
  }
  mng->current = current + 1;

  return current;
}

void
qhandler_delete(qhandler_manager_t *mng, qid_t id) {
  mng->handlers[id] = NULL;
}

qhandler_t*
qhandler_get(qhandler_manager_t *mng, qid_t id) {
  return mng->handlers[id];
}
