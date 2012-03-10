/**
 * Copyright (c) Xiaowei Wu
 */

#include "config.h"
#include "system.h"

int epoll_build(net_event_driver_t *event_driver)
{
    int                  e_fd;
    epoll_ctx_t         *ctx;
    struct epoll_event  *ev;

    ctx = pmalloc(event_driver->pool, sizeof(epoll_ctx_t));
    if (ctx == NULL) {
        log_error(event_driver->logger, errno, "pmalloc memory failed.");
        return EVENT_ERROR;
    }

    log_info(event_driver->logger, 0, "epoll size: %d", event_driver->size);

    e_fd = epoll_create(event_driver->size);   
    if (e_fd == -1) {
        log_error(event_driver->logger, errno, "create epoll fd failed.");
        return EVENT_ERROR;
    }

    ev = pmalloc(event_driver->pool, 
                 sizeof(struct epoll_event) * event_driver->size);
    if (ev == NULL) {
        log_error(event_driver->logger, errno, "pmalloc memory failed.");
        return EVENT_ERROR;
    }

    ctx->epoll_fd = e_fd;
    ctx->events = ev;

    event_driver->io_ctx = ctx;

    return EVENT_OK;
}

int epoll_destroy(net_event_driver_t *event_driver)
{
    epoll_ctx_t *ctx;

    ctx = event_driver->io_ctx;

    if (close(ctx->epoll_fd) == -1) {
        log_error(event_driver->logger, errno, "close epoll's fd failed.");
        return EVENT_ERROR;
    }

    return EVENT_OK;
}

int epoll_add_event(net_event_driver_t *event_driver, tcp_connection_t *conn,
        int events)
{
    int                 op;
    epoll_ctx_t        *ctx;
    struct epoll_event  ee;

    ctx = event_driver->io_ctx;

    op = conn->events == EV_NONE_EVENT ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    ee.events = 0;
    
    events |= conn->events;

    if (events & EV_READ_EVENT) {
        ee.events |= EPOLLIN;
    }

    if (events & EV_WRITE_EVENT) {
        ee.events |= EPOLLOUT;
    }

    ee.data.ptr = conn;

    if (epoll_ctl(ctx->epoll_fd, op, conn->conn_fd, &ee) == -1) {
        return EVENT_ERROR;
    }

    conn->events = events;

    return EVENT_OK;
}

int epoll_del_event(net_event_driver_t *event_driver, tcp_connection_t *conn,
        int events)
{
    epoll_ctx_t         *ctx;
    struct epoll_event   ee;

    ctx = event_driver->io_ctx;

    events = conn->events & (~events);
    
    ee.events = 0;

    if (events & EV_READ_EVENT) {
        ee.events |= EPOLLIN;
    }

    if (events & EV_WRITE_EVENT) {
        ee.events |= EPOLLOUT;
    }

    ee.data.ptr = conn;

    /* redis */
    if (events != 0) {
        epoll_ctl(ctx->epoll_fd, EPOLL_CTL_MOD, conn->conn_fd, &ee);
    } else {
        epoll_ctl(ctx->epoll_fd, EPOLL_CTL_DEL, conn->conn_fd, &ee);
    }

    conn->events = events;

    return EVENT_OK;
}

int epoll_polling(net_event_driver_t *event_driver)
{
    int                 event_num, i, active_events;
    epoll_ctx_t        *ctx;
    tcp_connection_t   *conn;
    struct epoll_event *ee;

    ctx = event_driver->io_ctx;

    event_num = epoll_wait(ctx->epoll_fd, ctx->events, event_driver->size, 
                           1000);

    if (event_num > 0) {
        for (i = 0; i < event_num; i++) {
            ee = ctx->events + i;

            active_events = EV_NONE_EVENT;

            if (ee->events & EPOLLIN) {
               active_events |= EV_READ_EVENT;
            }

            if (ee->events & EPOLLOUT) {
                active_events |= EV_WRITE_EVENT;
            }

            conn = (tcp_connection_t *) ee->data.ptr;
            conn->active_events = active_events;

            conn->next = event_driver->active_conns;
            event_driver->active_conns = conn;

#ifdef DEBUG
            if (conn->is_listen) {
                log_debug(event_driver->logger, 0,
                          "Events(read:%d) from listen socket (%d).",
                          (active_events & EV_READ_EVENT) != 0, conn->conn_fd);
            } else {
                log_debug(event_driver->logger, 0,
                          "Events(read:%d, write:%d) from connection with"
                          " client(addr:%s, port:%d)",
                          (active_events & EV_READ_EVENT) != 0,
                          (active_events & EV_WRITE_EVENT) != 0,
                          conn->client_addr.data,
                          conn->client_port);
            }
#endif
        } /* for loop */
    } /* if */

    return EVENT_OK;
}
