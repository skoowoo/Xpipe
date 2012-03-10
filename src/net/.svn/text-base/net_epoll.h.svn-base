/**
 * Copyright (c) Xiaowei Wu
 */

#ifndef __NET_EPOLL_H__
#define __NET_EPOLL_H__

#include "config.h"
#include "system.h"

typedef struct {
    int                 epoll_fd;
    struct epoll_event *events;
} epoll_ctx_t;

int epoll_build(net_event_driver_t *event_driver);
int epoll_destroy(net_event_driver_t *event_driver);
int epoll_add_event(net_event_driver_t *event_driver, tcp_connection_t *conn,
        int events);
int epoll_del_event(net_event_driver_t *event_driver, tcp_connection_t *conn,
        int events);
int epoll_polling(net_event_driver_t *event_driver);

#endif  /* __NET_EPOLL_H__ */
