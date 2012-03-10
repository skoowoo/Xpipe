/**
 * Copyright (c) Xiaowei Wu
 */

#ifndef __NET_EVENT_H__
#define __NET_EVENT_H__

#include "config.h"
#include "system.h"

#define EVENT_OK     0
#define EVENT_ERROR -1

#define EV_NONE_EVENT  0
#define EV_READ_EVENT  1
#define EV_WRITE_EVENT 2


typedef int (*ev_create_fp) (net_event_driver_t *event_driver);
typedef int (*ev_destroy_fp) (net_event_driver_t *event_driver);
typedef int (*ev_add_event_fp) (net_event_driver_t *event_driver,
        tcp_connection_t *conn, int events);
typedef int (*ev_delete_event_fp) (net_event_driver_t *event_driver,
        tcp_connection_t *conn, int events);
typedef int (*ev_event_poll_fp) (net_event_driver_t *event_driver);

typedef struct {
    ev_create_fp        create_handler;
    ev_destroy_fp       destroy_handler;
    ev_add_event_fp     add_handler;
    ev_delete_event_fp  del_handler;    
    ev_event_poll_fp    poll_handler;
} event_actions_t;

struct net_event_driver_s {
    void               *io_ctx;
    int                 size;
    event_actions_t    *actions;
    tcp_connection_t   *active_conns;
    mem_pool_t         *pool;
    logger_t           *logger;
};

extern event_actions_t event_actions;

int net_event_init(net_event_driver_t *event_driver);
int process_events(net_event_driver_t *event_driver);
int add_event(net_event_driver_t *event_driver, tcp_connection_t *conn, 
        int events, event_handler_fp handler);
int del_event(net_event_driver_t *event_driver, tcp_connection_t *conn, 
        int events);

#endif  /* __NET_EVENT_H__ */
