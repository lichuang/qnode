/*
 * See Copyright Notice in qnode.h
 */

#include <stdio.h>
#include "qassert.h"
#include "qevent.h"
#include "qmalloc.h"
#include "qminheap.h"

extern const struct qnode_event_op_t epoll_ops;

qnode_engine_t* qnode_engine_create(int size) {
    qnode_assert(size > 0);
    qnode_engine_t *engine = qnode_alloc_type(qnode_engine_t);
    engine->op = &epoll_ops;
    engine->data = engine->op->init(engine);
    INIT_LIST_HEAD(&(engine->event_list));
    INIT_LIST_HEAD(&(engine->active_list));
    qnode_minheap_ctor(&(engine->timeheap));
    return engine;
}

qnode_event_t* qnode_event_new(int fd, short events, qnode_event_fun_t *callback, void *data) {
    qnode_event_t *event = qnode_alloc_type(qnode_event_t);
    event->fd = fd;
    event->callback = callback;
    event->data = data;
    event->events = events;
    INIT_LIST_HEAD(&(event->active_entry));
    INIT_LIST_HEAD(&(event->event_entry));
    qnode_minheap_elem_init(event);
    return event;
}

static void event_queue_insert(qnode_engine_t *engine, qnode_event_t *event, int queue) {
    if (event->flags & queue) {
        if (queue & QEVENT_LIST_ACTIVE)
            return;
    }

    if (~event->flags & QEVENT_LIST_INTERNAL) {
        engine->event_count++;
    }
    event->flags |= queue;
    switch (queue) {
    case QEVENT_LIST_INSERTED:
        list_add_tail(&(event->event_entry), &(engine->event_list));
        break;
    case QEVENT_LIST_ACTIVE:
        engine->active_event_count++;
        list_add_tail(&(event->active_entry), &(engine->active_list));
        break;
    case QEVENT_LIST_TIMEOUT:
        qnode_minheap_push(&(engine->timeheap), event);
        break;
    default:
        break;
    }
}

static void event_queue_remove(qnode_engine_t *engine, qnode_event_t *event, int queue) {
    if (!(event->flags & queue)) {
    }

    if (~event->flags & QEVENT_LIST_INTERNAL) {
        engine->event_count--;
    }
    event->flags &= ~queue;
    switch (queue) {
    case QEVENT_LIST_INSERTED:
        list_del_init(&(event->event_entry));
        break;
    case QEVENT_LIST_ACTIVE:
        engine->active_event_count--;
        list_del_init(&(event->active_entry));
        break;
    case QEVENT_LIST_TIMEOUT:
        qnode_minheap_erase(&(engine->timeheap), event);
        break;
    default:
        break;
    }
}

static int get_time(qnode_engine_t* engine, struct timeval *time) {
    if (engine->time_cache.tv_sec) {
        *time = engine->time_cache;
        return (0);
    }

    return gettimeofday(time, NULL);
}

static void add_time(const struct timeval *time1, const struct timeval *time2, struct timeval *result) {
    result->tv_sec = time1->tv_sec + time2->tv_sec; 
    result->tv_usec = time1->tv_usec + time2->tv_usec; 
    if (result->tv_usec >= 1000000) {
        result->tv_sec++;
        result->tv_usec -= 1000000;
    }
}

static void sub_time(const struct timeval *time1, const struct timeval *time2, struct timeval *result) {
    result->tv_sec  = time1->tv_sec - time2->tv_sec;
    result->tv_usec = time1->tv_usec - time2->tv_usec;
    if (result->tv_usec < 0) {
        result->tv_sec--;
        result->tv_usec += 1000000;
    }
}

static int is_time_big(const struct timeval *time1, const struct timeval *time2) {
    return (time1->tv_sec == time2->tv_sec) ?
        (time1->tv_usec >= time2->tv_usec) :
        (time1->tv_sec  >= time2->tv_sec);
}

int is_time_small(const struct timeval *time1, const struct timeval *time2) {
    return is_time_big(time2, time1);
}

static void correct_timeout(qnode_engine_t *engine, struct timeval *time) {
    qnode_event_t **event;
    unsigned int size;
    struct timeval off;

    get_time(engine, time);
    if (is_time_big(time, &(engine->event_time))) {
        engine->event_time = *time;
        return;
    }
    sub_time(&(engine->event_time), time, &off);

    event = engine->timeheap.events;
    size  = engine->timeheap.size;
    for (; size > 0; ++event, --size) {
        struct timeval *event_timeout = &((*event)->timeout);
        sub_time(event_timeout, &off, event_timeout);
    }
    engine->event_time = *time;
}

static void clear_time(struct timeval *time) {
    time->tv_sec = time->tv_usec = 0;
}

static int next_timeout(qnode_engine_t *engine, struct timeval **ptime) {
    struct timeval now;
    qnode_event_t *event;
    struct timeval *time = *ptime;

    if ((event = qnode_minheap_top(&(engine->timeheap))) == NULL) {
        *ptime = NULL;
        return 0;
    }

    if (get_time(engine, &now) == -1) {
        return -1;
    }

    if (is_time_big(&now, &(event->timeout))) {
        clear_time(time);
        return 0;
    }

    sub_time(&(event->timeout), &now, time);

    qnode_assert(time->tv_sec >= 0);
    qnode_assert(time->tv_usec >= 0);

    return 0;
}

static int has_events(qnode_engine_t *engine) {
    return (engine->event_count > 0);
}

static void process_timeout(qnode_engine_t *engine) {
    struct timeval now;
    qnode_event_t *event;

    if (qnode_minheap_empty(&(engine->timeheap))) {
        return;
    }

    get_time(engine, &now);

    while ((event = qnode_minheap_top(&(engine->timeheap))) != NULL) {
        if (is_time_small(&(event->timeout), &now)) {
            break;
        }

        qnode_event_del(event);

        qnode_event_active(event, QEVENT_TIMEOUT, 1);
    }
}

static void process_active_event(qnode_engine_t *engine) {
    qnode_event_t *event;
    struct list_head *pos = NULL;
    struct list_head *active_list = &(engine->active_list);
    short ncalls;

    list_for_each(pos, active_list) {
        event = list_entry(pos, qnode_event_t, active_entry);
        if (event->events & QEVENT_PERSIST) {
            event_queue_remove(engine, event, QEVENT_LIST_ACTIVE);
        } else {
            qnode_event_del(event);
        }

        ncalls = event->ncalls;
        event->pncalls = &ncalls;
        while (ncalls) {
            ncalls--;
            event->ncalls = ncalls;
            (*event->callback)(event->fd, event->result, event->data);
        }
    }
}

int qnode_event_add(qnode_event_t *event, const struct timeval *time, qnode_engine_t *engine) {
    event->engine = engine;
    const qnode_event_op_t *ops = engine->op;
    void *data = engine->data;
    int result = 0;

    qnode_assert(!(event->flags & ~QEVENT_LIST_ALL));

    if (time != NULL && !(event->flags & QEVENT_LIST_TIMEOUT)) {
        if (qnode_minheap_reserve(&(engine->timeheap),
                1 + qnode_minheap_size(&engine->timeheap)) == -1) {
            return -1;
        }
    }

    if ((event->events & (QEVENT_READ|QEVENT_WRITE|QEVENT_SIGNAL)) &&
        !(event->flags & (QEVENT_LIST_INSERTED|QEVENT_LIST_ACTIVE))) {
        result = ops->add(data, event);
        if (result != -1) {
            event_queue_insert(engine, event, QEVENT_LIST_INSERTED);
        }
    }

    if (result != -1 && time != NULL) {
        struct timeval now;

        if (event->flags & QEVENT_LIST_TIMEOUT) {
            event_queue_remove(engine, event, QEVENT_LIST_TIMEOUT);
        }
        if ((event->flags & QEVENT_LIST_ACTIVE) && (event->result & QEVENT_TIMEOUT)) {
            event_queue_remove(engine, event, QEVENT_LIST_ACTIVE);
        }

        get_time(engine, &now);
        add_time(&now, time, &event->timeout);

        event_queue_insert(engine, event, QEVENT_LIST_TIMEOUT);
    }

    return result;
}

int qnode_engine_run(qnode_engine_t *engine) {
    const struct qnode_event_op_t *op = engine->op;
    void *base = engine->data;
    struct timeval time;
    struct timeval *ptime;
    int result, done;

    engine->time_cache.tv_sec = 0;
    done = 0;
    while (!done) {
        correct_timeout(engine, &time);
        ptime = &time;
        if (!engine->active_event_count) {
            next_timeout(engine, &ptime);
        } else {
            clear_time(&time);
        }

        if (!has_events(engine)) {
            return 1;
        }

        get_time(engine, &(engine->event_time));

        engine->time_cache.tv_sec = 0;

        result = op->poll(engine, base, ptime);

        if (result == -1) {
            return -1;
        }

        get_time(engine, &(engine->time_cache));

        process_timeout(engine);

        if (engine->active_event_count) {
            process_active_event(engine);
            if (!engine->active_event_count) {
                done = 1;
            }
        }
    }

    return 0;
}


void qnode_event_active(qnode_event_t *event, int result, short ncalls) {
    if (event->flags & QEVENT_LIST_ACTIVE) {
        event->result |= result;
        return;
    }

    event->result = result;
    event->ncalls = ncalls;
    event_queue_insert(event->engine, event, QEVENT_LIST_ACTIVE);
}

int qnode_event_del(qnode_event_t *event) {
    return 0;
}
