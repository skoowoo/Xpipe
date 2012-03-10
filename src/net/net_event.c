/**
 * Copyright (c) Xiaowei Wu
 */

#include "config.h"
#include "system.h"

event_actions_t event_actions = {
    epoll_build,
    epoll_destroy,
    epoll_add_event,
    epoll_del_event,
    epoll_polling
};


int net_event_init(net_event_driver_t *event_driver)
{
    event_actions_t *actions;

    actions = &event_actions;
    event_driver->actions = actions;
    event_driver->active_conns = NULL;

    if (actions->create_handler(event_driver) == EVENT_ERROR) {
        return EVENT_ERROR;
    }

    return EVENT_OK;
}

int process_events(net_event_driver_t *event_driver)
{
    event_actions_t *actions;
    tcp_connection_t *active_conn, *free_conn;

    actions = event_driver->actions;
    
    actions->poll_handler(event_driver);

    active_conn = event_driver->active_conns;

    while (active_conn) {
        if (active_conn->active_events & EV_READ_EVENT) {
            active_conn->read_event_handler(active_conn);
        }

        if (active_conn->active_events & EV_WRITE_EVENT) {
            active_conn->write_event_handler(active_conn);
        }


        if (active_conn->dead_events) {
#ifdef DEBUG
            log_debug(event_driver->logger, 0,
                      "Clean events(read:%d, write:%d) "
                      "on client(addr:%s, port:%d), connection(%d)",
                      (active_conn->dead_events & EV_READ_EVENT) != 0,
                      (active_conn->dead_events & EV_WRITE_EVENT) != 0,
                      active_conn->client_addr.data,
                      active_conn->client_port,
                      active_conn->conn_fd);
#endif

            del_event(event_driver, active_conn, active_conn->dead_events);
            active_conn->dead_events = EV_NONE_EVENT;
        }

        if (active_conn->close) {
#ifdef DEBUG
            log_debug(event_driver->logger, 0,
                      "Close connection(%d) with client(addr:%s, port:%d)",
                      active_conn->conn_fd,
                      active_conn->client_addr.data,
                      active_conn->client_port);
#endif

            close(active_conn->conn_fd);

            free_conn = active_conn;
            active_conn = active_conn->next;

            tcp_free_connection(free_conn->server, free_conn);
            continue;
        }

        if (active_conn->add_events) {
            add_event(event_driver, active_conn, active_conn->add_events,
                      tcp_server_send);
            active_conn->add_events = EV_NONE_EVENT;
        }

        /* next */
        active_conn = active_conn->next;
    }

    event_driver->active_conns = NULL;

    return EVENT_OK;
}

int add_event(net_event_driver_t *event_driver, tcp_connection_t *conn,
        int events, event_handler_fp handler)
{
    event_actions_t *actions;

    actions = event_driver->actions;

    if (actions->add_handler(event_driver, conn, events) == EVENT_ERROR) { 
        return EVENT_ERROR;
    }

    if (events & EV_READ_EVENT) {
       conn->read_event_handler = handler;
    } 
    
    if (events &EV_WRITE_EVENT) {
        conn->write_event_handler = handler;
    }

    return EVENT_OK;
}

int del_event(net_event_driver_t *event_driver, tcp_connection_t *conn,
        int events)
{
    event_actions_t *actions;

    actions = event_driver->actions;

    if (conn->events == EV_NONE_EVENT) {
        return EVENT_OK;
    }

    if (actions->del_handler(event_driver, conn, events) == EVENT_ERROR) {
        return EVENT_ERROR;
    }

    return EVENT_OK;
}


