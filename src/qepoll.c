/*
 * See Copyright Notice in qnode.h
 */

#include <sys/epoll.h>
#include "qevent.h"
#include "qlog.h"
#include "qmalloc.h"

#define INITIAL_NFILES 32
#define INITIAL_NEVENTS 32
#define MAX_NEVENTS 4096

static void *epoll_init (struct qnode_dispatcher_t *dispatcher);
static int epoll_add    (void *, struct qnode_event_t *);
static int epoll_del    (void *, struct qnode_event_t *);
static int epoll_dispatch(qnode_dispatcher_t *dispatcher, void *arg, struct timeval *tv);

const struct qnode_event_op_t epoll_ops = {
  "epoll",
  epoll_init,
  epoll_add,
  epoll_del,
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

static void* epoll_init(struct qnode_dispatcher_t *dispatcher) {
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

  epollop->events = qnode_alloc_array(INITIAL_NEVENTS * sizeof(struct epoll_event));
  if (epollop->events == NULL) {
    free(epollop);
    return (NULL);
  }
  epollop->nevents = INITIAL_NEVENTS;

  epollop->fds = qnode_alloc_array(INITIAL_NFILES * sizeof(struct evepoll));
  if (epollop->fds == NULL) {
    free(epollop->events);
    free(epollop);
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
            nfds << = 1;
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
    if (ev->events & EV_READ) {
        events |= EPOLLIN;
    }
    if (ev->events & EV_WRITE) {
        events |= EPOLLOUT;
    }
    epev.data.fd = fd;
    epev.events = events;
    if (epoll_ctl(epollop->epfd, op, ev->ev_fd, &epev) == -1) {
        return (-1);
    }
    if (ev->ev_events & EV_READ) {
        evep->evread = event;
    }
    if (ev->ev_events & EV_WRITE) {
        evep->evwrite = event;
    }
}

static int epoll_del(void *, struct qnode_event_t *) {
}

static int epoll_dispatch(qnode_dispatcher_t *dispatcher, void *arg, struct timeval *tv) {
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
            event_warn("epoll_wait");
            return (-1);
        }

        evsignal_process(base);
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
            read = evep->read;
            write = evep->write;
        } else {
            if (what & EPOLLIN) {
                read = evep->read;
            }

            if (what & EPOLLOUT) {
                write = evep->write;
            }
        }

        if (!(read || write)) {
            continue;
        }

        if (read != NULL) {
            event_active(evread, EV_READ, 1);
        }
        if (write != NULL) {
            event_active(evwrite, EV_WRITE, 1);
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
