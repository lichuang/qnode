/*
 * See Copyright Notice in qnode.h
 */

#include <errno.h>
#include <string.h>
#include <sys/epoll.h>
#include "qevent.h"
#include "qlog.h"
#include "qmalloc.h"

#define INITIAL_NFILES 32
#define INITIAL_NEVENTS 32
#define MAX_NEVENTS 4096
#define MAX_EPOLL_TIMEOUT_MSEC (35*60*1000)

static void *epoll_init (struct qnode_engine_t *engine);
static int epoll_add    (void *, struct qnode_event_t *);
static int epoll_del    (void *, struct qnode_event_t *);
static int epoll_dispatch(qnode_engine_t *engine, void *arg, struct timeval *tv);

const struct qnode_event_op_t epoll_ops = {
  "epoll",
  epoll_init,
  epoll_add,
  epoll_del,
  epoll_dispatch,
};

struct evepoll {
  struct qnode_event_t *read_event;
  struct qnode_event_t *write_event;
};

struct epollop {
  struct evepoll *fds;
  int nfds;
  struct epoll_event *events;
  int nevents;
  int epfd;
};

static void* epoll_init(struct qnode_engine_t *engine) {
  int epfd;
  struct epollop *epollop;

  if ((epfd = epoll_create(32000)) == -1) {
    return NULL;
  }

  //FD_CLOSEONEXEC(epfd);

  if (!(epollop = qnode_alloc_type(struct epollop))) {
    return NULL;
  }

  epollop->epfd = epfd;

  epollop->events = qnode_alloc_array(struct epoll_event, INITIAL_NEVENTS * sizeof(struct epoll_event));
  if (epollop->events == NULL) {
    qnode_free(epollop);
    return (NULL);
  }
  epollop->nevents = INITIAL_NEVENTS;

  epollop->fds = qnode_alloc_array(struct evepoll, INITIAL_NFILES * sizeof(struct evepoll));
  if (epollop->fds == NULL) {
    qnode_free(epollop->events);
    qnode_free(epollop);
    return (NULL);
  }
  epollop->nfds = INITIAL_NFILES;

  return (epollop);
}

static int epoll_recalc(void *arg, int max) {
    struct epollop *epollop = arg;

    if (max >= epollop->nfds) {
        struct evepoll *fds;
        int nfds;

        nfds = epollop->nfds;
        while (nfds <= max) {
            nfds <<= 1;
        }
        fds = qnode_realloc(epollop->fds, nfds * sizeof(struct evepoll));
        if (fds == NULL) {
            return -1;
        }
        epollop->fds = fds;
        memset(fds + epollop->nfds, 0, (nfds - epollop->nfds) * sizeof(struct evepoll));
        epollop->nfds = nfds;
    }
    return 0;
}

static int epoll_add(void *arg, struct qnode_event_t *event) {
    struct epollop *epollop = arg;
    struct epoll_event epev = {0, {0}};
    struct evepoll *evep;
    int fd, op, events;

    fd = event->fd;
    if (fd >= epollop->nfds) {
      if (epoll_recalc(epollop, fd) == -1) {
        return (-1);
      }
    }
    evep = &(epollop->fds[fd]);
    op = EPOLL_CTL_ADD;
    events = 0;
    if (evep->read_event != NULL) {
        events |= EPOLLIN;
        op = EPOLL_CTL_MOD;
    }
    if (evep->write_event != NULL) {
        events |= EPOLLOUT;
        op = EPOLL_CTL_MOD;
    }
    if (event->events & QEVENT_READ) {
        events |= EPOLLIN;
    }
    if (event->events & QEVENT_WRITE) {
        events |= EPOLLOUT;
    }
    epev.data.fd = fd;
    epev.events = events;
    if (epoll_ctl(epollop->epfd, op, event->fd, &epev) == -1) {
        return -1;
    }
    if (event->events & QEVENT_READ) {
        evep->read_event = event;
    }
    if (event->events & QEVENT_WRITE) {
        evep->write_event = event;
    }
    return 0;
}

static int epoll_del(void *arg, struct qnode_event_t *event) {
  return 1;
}

static int epoll_dispatch(qnode_engine_t *engine, void *arg, struct timeval *tv) {
    struct epollop *epollop = arg;
    struct epoll_event *events = epollop->events;
    struct evepoll *evep;
    int i, res, timeout = -1;

    if (tv != NULL) {
        timeout = tv->tv_sec * 1000 + (tv->tv_usec + 999) / 1000;
    }

    if (timeout > MAX_EPOLL_TIMEOUT_MSEC) {
        timeout = MAX_EPOLL_TIMEOUT_MSEC;
    }

    res = epoll_wait(epollop->epfd, events, epollop->nevents, timeout);

    if (res == -1) {
        if (errno != EINTR) {
            return (-1);
        }

        return (0);
    }

    for (i = 0; i < res; i++) {
        int what = events[i].events;
        struct qnode_event_t *read = NULL, *write = NULL;
        int fd = events[i].data.fd;

        if (fd < 0 || fd >= epollop->nfds)
            continue;
        evep = &epollop->fds[fd];

        if (what & (EPOLLHUP|EPOLLERR)) {
            read = evep->read_event;
            write = evep->write_event;
        } else {
            if (what & EPOLLIN) {
                read = evep->read_event;
            }

            if (what & EPOLLOUT) {
                write = evep->write_event;
            }
        }

        if (!(read || write)) {
            continue;
        }

        if (read != NULL) {
            qnode_event_active(read, QEVENT_READ, 1);
        }
        if (write != NULL) {
            qnode_event_active(write, QEVENT_WRITE, 1);
        }
    }

    if (res == epollop->nevents && epollop->nevents < MAX_NEVENTS) {
        int new_nevents = epollop->nevents * 2;
        struct epoll_event *new_events;

        new_events = qnode_realloc(epollop->events,
            new_nevents * sizeof(struct epoll_event));
        if (new_events) {
            epollop->events = new_events;
            epollop->nevents = new_nevents;
        }
    }

    return (0);
}
