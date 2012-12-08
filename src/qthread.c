/*
 * See Copyright Notice in qnode.h
 */

#include "qengine.h"
#include "qlog.h"
#include "qmalloc.h"
#include "qthread.h"

qnode_thread_t* qnode_thread_new() {
    qnode_thread_t *thread = qnode_alloc_type(qnode_thread_t);
    if (thread == NULL) {
        qnode_error("create thread error");
        return NULL;
    }
    thread->engine = qnode_engine_new();
    if (thread->engine == NULL) {
        qnode_error("create thread engine error");
        qnode_free(thread);
        return NULL;
    }

    return thread;
}
